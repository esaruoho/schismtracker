/*
 * Schism Tracker - a cross-platform Impulse Tracker clone
 *
 * Ableton Link + Link Audio.
 *
 * Link shares a beat timeline, a tempo and a transport state between programs on
 * the same network; Link Audio additionally streams audio between them. Both are
 * reached through Link's own C API (link/extensions/abl_link), so nothing here is
 * C++ and the rest of schism does not have to know that C++ is involved at all.
 *
 * Everything in this header is safe to call whether or not Link was compiled in:
 * without USE_LINK the whole thing is inert and reports itself unavailable.
 *
 * FEATURE-CARD >> features/ableton-link.feature
 */

#ifndef SCHISM_LINK_H_
#define SCHISM_LINK_H_

#include "headers.h"

/* Runtime switches, persisted in the config. These are meaningful even in a build
 * without Link -- they just cannot be turned on. */
#define LINK_FLAG_ENABLED       0x01 /* join the Link session at all */
#define LINK_FLAG_TEMPO_FOLLOW  0x02 /* take our tempo from the session */
#define LINK_FLAG_STARTSTOP     0x04 /* share transport start/stop */
#define LINK_FLAG_AUDIO_SEND    0x08 /* publish our output as a Link Audio channel */

extern int link_flags;

/* True if this build can do Link at all. */
int link_available(void);

/* Called once at startup / shutdown from the same places the MIDI core is set up. */
void link_init(void);
void link_quit(void);

/* Apply link_flags: joins or leaves the session, creates or drops the audio sink.
 * Safe to call repeatedly; only acts on what actually changed. Not realtime-safe --
 * call it from the UI, never from the audio callback. */
void link_apply_flags(void);

/* Called from the main loop. Reads the peer count, and performs transport sync --
 * song_start()/song_stop() are not remotely realtime-safe, so the audio thread only
 * notices the change and this acts on it. */
void link_poll(void);

/* How many other Link peers are visible, or 0. For the status line. */
int link_num_peers(void);

/* The session tempo, or 0.0 if unavailable. */
double link_session_tempo(void);

/* Push our tempo out to the session (when we are NOT following). */
void link_set_tempo(double bpm);

/* Tell the session we started or stopped, for start/stop sync. */
void link_set_playing(int playing);

/* --- audio-thread half ---------------------------------------------------
 * Called from the audio callback, around the mixdown. Realtime-safe: no
 * allocation, no locks that the Link docs do not already declare safe.
 *
 * link_audio_begin() captures the session state, and is where tempo follow and
 * transport sync are actually applied.
 * link_audio_end() hands the rendered buffer to Link Audio, if we are publishing.
 * They must be called in that order, once each per callback. */
void link_audio_begin(uint32_t frames, uint32_t sample_rate);
/* `bits` is the REAL output width (8/16/24/32/64) and `is_float` says whether those
 * are floats -- the sink takes interleaved int16, so anything else is converted. */
void link_audio_end(const void *buffer, uint32_t frames, uint32_t channels,
	uint32_t bits, int is_float, uint32_t sample_rate);

/* Link Audio publication counters, for the status line: how many buffers we have
 * committed, and for each way it can fail, how often. */
void link_audio_stats(unsigned long *commits, unsigned long *no_buffer,
	unsigned long *too_small, unsigned long *bad_format);

#endif /* SCHISM_LINK_H_ */
