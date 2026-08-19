/*
 * Schism Tracker - a cross-platform Impulse Tracker clone
 *
 * Ableton Link + Link Audio.  See include/link.h for the contract.
 * FEATURE-CARD >> features/ableton-link.feature
 *
 * Layout of this file mirrors the two threads it serves:
 *
 *   app thread   link_init / link_quit / link_apply_flags / link_poll
 *                -- joins and leaves the session, creates the audio sink, and
 *                   does transport sync, because song_start()/song_stop() are
 *                   nowhere near realtime-safe.
 *
 *   audio thread link_audio_begin / link_audio_end
 *                -- captures the session state, applies tempo follow, and hands
 *                   the rendered buffer to Link Audio. Only calls things Link
 *                   documents as realtime-safe.
 */

#include "headers.h"
#include "link.h"
#include "song.h"
#include "log.h"
#include "player/sndfile.h"

/* Off unless the user asks, every time. Joining a network session and announcing
 * an audio channel are both things that should never happen by surprise. */
int link_flags = 0;

#ifndef USE_LINK

int link_available(void) { return 0; }
void link_init(void) {}
void link_quit(void) {}
void link_apply_flags(void) { link_flags = 0; }
void link_poll(void) {}
int link_num_peers(void) { return 0; }
double link_session_tempo(void) { return 0.0; }
void link_set_tempo(double bpm) { (void)bpm; }
void link_set_playing(int playing) { (void)playing; }
void link_audio_begin(uint32_t frames, uint32_t sample_rate)
	{ (void)frames; (void)sample_rate; }
void link_audio_end(const void *buffer, uint32_t frames, uint32_t channels,
	uint32_t bits, int is_float, uint32_t sample_rate)
	{ (void)buffer; (void)frames; (void)channels; (void)bits; (void)is_float;
	  (void)sample_rate; }
void link_audio_stats(unsigned long *a, unsigned long *b, unsigned long *c,
	unsigned long *d) { if (a) *a = 0; if (b) *b = 0; if (c) *c = 0; if (d) *d = 0; }

#else /* USE_LINK */

#include <abl_link.h>

/* One bar of four beats. IT has no notion of a bar, so this is simply the grid we
 * quantise to when joining, and what Link Audio is told our buffers sit on. */
#define LINK_QUANTUM 4.0

/* IT's tempo is an integer BPM in this range, which is the hard limit on how
 * closely we can follow a Link session -- see the feature card. */
#define LINK_TEMPO_MIN 31
#define LINK_TEMPO_MAX 255

static struct abl_link g_link = { NULL };
static struct abl_link_audio_sink g_sink = { NULL };
static int g_joined = 0;
static int g_sink_up = 0;

/* Written by the audio thread, read by the app thread. Only ever a snapshot for
 * the status line, so a torn read costs nothing. */
static volatile int g_peers = 0;
static volatile double g_tempo = 0.0;

/* Transport sync hand-off: the audio thread sees Link's playing state change, the
 * app thread is the one allowed to act on it. -1 = nothing pending. */
static volatile int g_want_playing = -1;
/* What we last told Link, so we do not fight our own announcement. */
static int g_told_playing = -1;

/* Session state is documented "Thread-safe: no", so the two threads get one each.
 * Sharing a single object between the audio callback and the UI was a race, and the
 * reason enabling the audio send crashed. */
static abl_link_session_state g_state_audio = { NULL };  /* audio thread only */
static abl_link_session_state g_state_app = { NULL };    /* app thread only */
static double g_beat_at_buffer = 0.0;
static int g_state_valid = 0;

/* Conversion scratch for output formats that are not already int16. Static so the
 * audio callback never allocates; 64K samples is far more than any buffer schism
 * hands us, and a bigger one is skipped rather than overrunning this. */
#define LINK_CONV_SAMPLES 65536
static int16_t g_conv[LINK_CONV_SAMPLES];
/* Last size we asked the sink for, so we only ask when it changes. */
static size_t g_conv_requested = 0;

/* Publication counters, for the status line. "The channel is listed but records
 * silence" is otherwise unanswerable: these say whether we are committing audio at
 * all, and if not, which step is refusing. */
static volatile unsigned long g_commits = 0;   /* buffers handed to the sink */
static volatile unsigned long g_norefuse = 0;  /* retain_buffer gave us nothing */
static volatile unsigned long g_toosmall = 0;  /* buffer smaller than our frame */
static volatile unsigned long g_badfmt = 0;    /* output format we cannot convert */

int link_available(void)
{
	return 1;
}

void link_init(void)
{
	/* Nothing is created here on purpose: creating the Link instance IS joining
	 * the session and starting network discovery, and that must wait until the
	 * user turns it on. */
	g_state_audio = abl_link_create_session_state();
	g_state_app = abl_link_create_session_state();
}

static void link_sink_down(void)
{
	if (g_sink_up) {
		abl_link_audio_sink_destroy(g_sink);
		g_sink.impl = NULL;
		g_sink_up = 0;
		g_conv_requested = 0;
	}
}

static void link_leave(void)
{
	link_sink_down();
	if (g_joined) {
		abl_link_enable(g_link, false);
		abl_link_destroy(g_link);
		g_link.impl = NULL;
		g_joined = 0;
		g_peers = 0;
		g_tempo = 0.0;
		g_told_playing = -1;
		g_want_playing = -1;
	}
}

void link_quit(void)
{
	/* Same reason as link_apply_flags: the callback must not be inside the sink or
	 * the Link instance while they are torn down. */
	song_lock_audio();
	link_leave();
	song_unlock_audio();

	if (g_state_audio.impl) {
		abl_link_destroy_session_state(g_state_audio);
		g_state_audio.impl = NULL;
	}
	if (g_state_app.impl) {
		abl_link_destroy_session_state(g_state_app);
		g_state_app.impl = NULL;
	}
}

/* The sink and the Link instance are touched by the audio callback, and Link
 * documents the sink calls as NOT thread-safe. So every mutation here happens with
 * the audio lock held: the callback cannot be inside link_audio_begin/end while we
 * create or destroy them. Creating a sink from the UI while the callback was using
 * it is what crashed on Esa's machine the moment "Link Audio out" went on. */
void link_apply_flags(void)
{
	int want = !!(link_flags & LINK_FLAG_ENABLED);
	double joined_now = 0.0;
	int sink_msg = 0;

	song_lock_audio();

	if (!want) {
		int was = g_joined;
		link_leave();
		song_unlock_audio();
		if (was)
			log_appendf(5, " Ableton Link disabled");
		return;
	}

	if (!g_joined) {
		double bpm = current_song ? (double)current_song->current_tempo : 125.0;

		g_link = abl_link_create(bpm);
		if (!g_link.impl) {
			link_flags &= ~LINK_FLAG_ENABLED;
			song_unlock_audio();
			log_appendf(4, " Ableton Link: could not start");
			return;
		}
		abl_link_enable(g_link, true);
		/* Named, so we show up as something recognisable rather than blank in
		 * whatever is listening. */
		abl_link_audio_set_peer_name(g_link, "Schism Tracker");
		g_joined = 1;
		joined_now = bpm;
	}

	abl_link_enable_start_stop_sync(g_link, !!(link_flags & LINK_FLAG_STARTSTOP));

	/* Link Audio is a separate opt-in on top of the session: it announces a
	 * channel other programs can listen to, which is a bigger thing to do to a
	 * network than sharing a tempo. */
	abl_link_audio_enable_link_audio(g_link, !!(link_flags & LINK_FLAG_AUDIO_SEND));

	if (link_flags & LINK_FLAG_AUDIO_SEND) {
		if (!g_sink_up) {
			/* Generous: schism's buffer is chosen by the audio driver and can
			 * be re-sized under us, and this is stereo interleaved. */
			g_sink = abl_link_audio_sink_create(g_link, "Schism Tracker", 65536);
			if (g_sink.impl) {
				g_sink_up = 1;
				sink_msg = 1;
			} else {
				link_flags &= ~LINK_FLAG_AUDIO_SEND;
				sink_msg = -1;
			}
		}
	} else {
		link_sink_down();
	}

	song_unlock_audio();

	/* Messages after the unlock: log_appendf draws, and holding the audio lock
	 * across UI work is asking for the kind of stall that sounds like a dropout. */
	if (joined_now > 0.0)
		log_appendf(5, " Ableton Link enabled (tempo %.0f)", joined_now);
	if (sink_msg > 0)
		log_appendf(5, " Link Audio: publishing \"Schism Tracker\"");
	else if (sink_msg < 0)
		log_appendf(4, " Link Audio: could not create the channel");
}

int link_num_peers(void)
{
	return g_joined ? g_peers : 0;
}

double link_session_tempo(void)
{
	return g_joined ? g_tempo : 0.0;
}

void link_set_tempo(double bpm)
{
	if (!g_joined || (link_flags & LINK_FLAG_TEMPO_FOLLOW))
		return;

	abl_link_capture_app_session_state(g_link, g_state_app);
	abl_link_set_tempo(g_state_app, bpm, abl_link_clock_micros(g_link));
	abl_link_commit_app_session_state(g_link, g_state_app);
}

void link_set_playing(int playing)
{
	if (!g_joined || !(link_flags & LINK_FLAG_STARTSTOP))
		return;
	if (g_told_playing == !!playing)
		return;

	g_told_playing = !!playing;

	abl_link_capture_app_session_state(g_link, g_state_app);
	abl_link_set_is_playing(g_state_app, !!playing, abl_link_clock_micros(g_link));
	abl_link_commit_app_session_state(g_link, g_state_app);
}

/* App thread. Acts on anything the audio thread noticed. */
void link_poll(void)
{
	int want, playing;

	if (!g_joined)
		return;

	g_peers = (int)abl_link_num_peers(g_link);

	want = g_want_playing;
	if (want < 0)
		return;
	g_want_playing = -1;

	if (!(link_flags & LINK_FLAG_STARTSTOP))
		return;

	playing = (song_get_mode() & (MODE_PLAYING | MODE_PATTERN_LOOP)) ? 1 : 0;
	if (want == playing)
		return;

	/* Remember it as ours so link_set_playing does not echo it straight back. */
	g_told_playing = want;

	if (want)
		song_start();
	else
		song_stop();
}

/* --- audio thread ------------------------------------------------------- */

void link_audio_begin(uint32_t frames, uint32_t sample_rate)
{
	double tempo, beat;
	int itempo, playing;

	g_state_valid = 0;

	if (!g_joined || !g_state_audio.impl || !sample_rate)
		return;

	abl_link_capture_audio_session_state(g_link, g_state_audio);
	g_state_valid = 1;

	{
		const int64_t now = abl_link_clock_micros(g_link);

		tempo = abl_link_tempo(g_state_audio);
		g_tempo = tempo;

		beat = abl_link_beat_at_time(g_state_audio, now, LINK_QUANTUM);
		g_beat_at_buffer = beat;

		if (link_flags & LINK_FLAG_STARTSTOP) {
			playing = abl_link_is_playing(g_state_audio) ? 1 : 0;
			if (playing != g_told_playing)
				g_want_playing = playing;   /* the app thread acts */
		}

		if ((link_flags & LINK_FLAG_TEMPO_FOLLOW) && current_song) {
			/* IT tempo is an integer BPM, so this is the closest we can sit to
			 * the session; the residual is documented on the feature card. */
			itempo = (int)(tempo + 0.5);
			if (itempo >= LINK_TEMPO_MIN && itempo <= LINK_TEMPO_MAX
				&& itempo != (int)current_song->current_tempo)
				song_set_current_tempo(itempo);
		}
	}

	(void)frames;
}

/* Convert `count` interleaved samples of schism's output into int16 in `dst`.
 * Returns 0 if the format is not one we know how to read.
 *
 * The sink takes int16, and schism can be set to 8, 16, 24 or 32 bit, integer or
 * float. Publishing only at 16 bit was a silent trap: the channel appeared on the
 * network and stayed quiet, with nothing saying why. */
static int link_to_s16(int16_t *dst, const void *src, size_t count,
	uint32_t bits, int is_float)
{
	size_t i;

	if (is_float) {
		if (bits == 32) {
			const float *f = (const float *)src;
			for (i = 0; i < count; i++) {
				double v = f[i] * 32767.0;
				dst[i] = (int16_t)((v > 32767.0) ? 32767.0
					: (v < -32768.0) ? -32768.0 : v);
			}
			return 1;
		}
		if (bits == 64) {
			const double *d = (const double *)src;
			for (i = 0; i < count; i++) {
				double v = d[i] * 32767.0;
				dst[i] = (int16_t)((v > 32767.0) ? 32767.0
					: (v < -32768.0) ? -32768.0 : v);
			}
			return 1;
		}
		return 0;
	}

	switch (bits) {
	case 8: {
		/* schism's 8-bit output is UNSIGNED, so centre it and scale up. */
		const uint8_t *u = (const uint8_t *)src;
		for (i = 0; i < count; i++)
			dst[i] = (int16_t)(((int)u[i] - 128) << 8);
		return 1;
	}
	case 16:
		memcpy(dst, src, count * sizeof(int16_t));
		return 1;
	case 24: {
		/* three bytes per sample, little-endian; keep the top 16 bits */
		const uint8_t *b = (const uint8_t *)src;
		for (i = 0; i < count; i++)
			dst[i] = (int16_t)(b[i * 3 + 1] | (b[i * 3 + 2] << 8));
		return 1;
	}
	case 32: {
		const int32_t *l = (const int32_t *)src;
		for (i = 0; i < count; i++)
			dst[i] = (int16_t)(l[i] >> 16);
		return 1;
	}
	}
	return 0;
}

void link_audio_end(const void *buffer, uint32_t frames, uint32_t channels,
	uint32_t bits, int is_float, uint32_t sample_rate)
{
	struct abl_link_audio_sink_buffer_handle handle;
	const int16_t *out;
	size_t want;

	if (!g_sink_up || !g_state_valid || !buffer || !frames || !sample_rate)
		return;

	want = (size_t)frames * (size_t)channels;
	if (want > LINK_CONV_SAMPLES)
		return;                 /* absurd buffer; skip rather than overrun */

	if (bits == 16 && !is_float) {
		out = (const int16_t *)buffer;   /* already what the sink wants */
	} else {
		if (!link_to_s16(g_conv, buffer, want, bits, is_float)) {
			g_badfmt++;
			return;                     /* unknown format */
		}
		out = g_conv;
	}

	/* Tell the sink how much room we need, whenever that changes. This used to sit
	 * only on the "handle was valid but too small" path -- which meant that if the
	 * sink handed out NO buffer, it was never asked for one either, and the channel
	 * stayed visible on the network and permanently silent. Request first, ask
	 * second. Documented realtime-safe. */
	if (want != g_conv_requested) {
		abl_link_audio_sink_request_max_num_samples(g_sink, want);
		g_conv_requested = want;
	}

	handle = abl_link_audio_sink_retain_buffer(g_sink);
	if (!abl_link_audio_sink_buffer_is_valid(&handle)) {
		g_norefuse++;
		return;                 /* nobody listening yet, or no buffer free */
	}

	if (handle.max_num_samples < want || !handle.samples) {
		/* Skip this buffer rather than truncate it: a short commit would be heard
		 * as a gap. The request above will have grown it by the next callback. */
		abl_link_audio_sink_buffer_release(&handle);
		g_toosmall++;
		return;
	}

	memcpy(handle.samples, out, want * sizeof(int16_t));

	if (abl_link_audio_sink_buffer_commit(&handle, g_state_audio, g_beat_at_buffer,
			LINK_QUANTUM, (size_t)frames, (size_t)channels, sample_rate))
		g_commits++;
}

void link_audio_stats(unsigned long *commits, unsigned long *no_buffer,
	unsigned long *too_small, unsigned long *bad_format)
{
	if (commits)    *commits    = g_commits;
	if (no_buffer)  *no_buffer  = g_norefuse;
	if (too_small)  *too_small  = g_toosmall;
	if (bad_format) *bad_format = g_badfmt;
}

#endif /* USE_LINK */
