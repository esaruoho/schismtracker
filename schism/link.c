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
	uint32_t bits, uint32_t sample_rate)
	{ (void)buffer; (void)frames; (void)channels; (void)bits; (void)sample_rate; }

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

/* Session state captured for this callback, and the beat it starts on. */
static abl_link_session_state g_state = { NULL };
static double g_beat_at_buffer = 0.0;
static int g_state_valid = 0;

int link_available(void)
{
	return 1;
}

void link_init(void)
{
	/* Nothing is created here on purpose: creating the Link instance IS joining
	 * the session and starting network discovery, and that must wait until the
	 * user turns it on. */
	g_state = abl_link_create_session_state();
}

static void link_sink_down(void)
{
	if (g_sink_up) {
		abl_link_audio_sink_destroy(g_sink);
		g_sink.impl = NULL;
		g_sink_up = 0;
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
	link_leave();
	if (g_state.impl) {
		abl_link_destroy_session_state(g_state);
		g_state.impl = NULL;
	}
}

void link_apply_flags(void)
{
	int want = !!(link_flags & LINK_FLAG_ENABLED);

	if (!want) {
		if (g_joined)
			log_appendf(5, " Ableton Link disabled");
		link_leave();
		return;
	}

	if (!g_joined) {
		double bpm = current_song ? (double)current_song->current_tempo : 125.0;

		g_link = abl_link_create(bpm);
		if (!g_link.impl) {
			log_appendf(4, " Ableton Link: could not start");
			link_flags &= ~LINK_FLAG_ENABLED;
			return;
		}
		abl_link_enable(g_link, true);
		g_joined = 1;
		log_appendf(5, " Ableton Link enabled (tempo %.0f)", bpm);
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
				log_appendf(5, " Link Audio: publishing \"Schism Tracker\"");
			} else {
				log_appendf(4, " Link Audio: could not create the channel");
				link_flags &= ~LINK_FLAG_AUDIO_SEND;
			}
		}
	} else {
		link_sink_down();
	}
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

	abl_link_capture_app_session_state(g_link, g_state);
	abl_link_set_tempo(g_state, bpm, abl_link_clock_micros(g_link));
	abl_link_commit_app_session_state(g_link, g_state);
}

void link_set_playing(int playing)
{
	if (!g_joined || !(link_flags & LINK_FLAG_STARTSTOP))
		return;
	if (g_told_playing == !!playing)
		return;

	g_told_playing = !!playing;

	abl_link_capture_app_session_state(g_link, g_state);
	abl_link_set_is_playing(g_state, !!playing, abl_link_clock_micros(g_link));
	abl_link_commit_app_session_state(g_link, g_state);
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

	if (!g_joined || !g_state.impl || !sample_rate)
		return;

	abl_link_capture_audio_session_state(g_link, g_state);
	g_state_valid = 1;

	{
		const int64_t now = abl_link_clock_micros(g_link);

		tempo = abl_link_tempo(g_state);
		g_tempo = tempo;

		beat = abl_link_beat_at_time(g_state, now, LINK_QUANTUM);
		g_beat_at_buffer = beat;

		if (link_flags & LINK_FLAG_STARTSTOP) {
			playing = abl_link_is_playing(g_state) ? 1 : 0;
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

void link_audio_end(const void *buffer, uint32_t frames, uint32_t channels,
	uint32_t bits, uint32_t sample_rate)
{
	struct abl_link_audio_sink_buffer_handle handle;
	size_t want;

	if (!g_sink_up || !g_state_valid || !buffer || !frames || !sample_rate)
		return;

	/* The sink takes interleaved int16. Schism can be configured for 8 or 32 bit
	 * output, and converting here would mean a scratch buffer and a conversion in
	 * the audio callback for a case nobody sends over the network on purpose --
	 * so publish only when we are already producing what the sink wants. */
	if (bits != 16)
		return;

	want = (size_t)frames * (size_t)channels;

	handle = abl_link_audio_sink_retain_buffer(g_sink);
	if (!abl_link_audio_sink_buffer_is_valid(&handle))
		return;

	if (handle.max_num_samples < want || !handle.samples) {
		/* Ask for more room; this buffer is skipped rather than truncated,
		 * because a short commit would be heard as a gap. */
		abl_link_audio_sink_buffer_release(&handle);
		abl_link_audio_sink_request_max_num_samples(g_sink, want);
		return;
	}

	memcpy(handle.samples, buffer, want * sizeof(int16_t));

	abl_link_audio_sink_buffer_commit(&handle, g_state, g_beat_at_buffer,
		LINK_QUANTUM, (size_t)frames, (size_t)channels, sample_rate);
}

#endif /* USE_LINK */
