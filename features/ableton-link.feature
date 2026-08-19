# =============================================================================
# WIKI PAGE / REPORT CARD: Ableton Link + Link Audio
# Convention: GHERKIN-FEATURE-WIKI-PATTERN.md (as used in esaruoho/impulse-tracker)
#
# Asked for 2026-08-19 after someone donated and said: "Could you imagine having
# Impulse/Schism Tracker with Ableton Link or even Link Audio??? I would kill for
# that." So: shared tempo, shared transport, and schism's output published as an
# audio channel on the network.
#
# WHAT THIS CARD SPAWNS (generative SEED):
#   - CODESPACE  : schism/link.c + include/link.h, the --enable-link plumbing, the
#                  link/ submodule, and the four call sites that reach it.
#   - THINKSPACE : why the C++ stays in exactly one translation unit, which thread
#                  is allowed to do what, and the tempo-resolution limit.
#   - AREASPACE  : owns network tempo/transport/audio sharing. Must NOT change the
#                  mixer, and must be inert in a default build.
#
# Report-card legend:
#   @shipped        - in the fork
#   @build-verified - both configurations build clean
#   @lib-verified   - proven at runtime against the vendored library itself
#   @hw-untested    - not yet confirmed against real Link software
#
# Source files linked back to this card:
#   include/link.h            - the whole contract, inert without USE_LINK
#   schism/link.c             - app-thread half and audio-thread half
#   configure.ac              - --enable-link, the C++17 probe, platform defines
#   Makefile.am               - link.c always built; abl_link.cpp only if enabled
#   schism/audio_playback.c   - link_audio_begin / link_audio_end in the callback
#   schism/main.c             - link_init / link_quit / link_poll
#   schism/midi-core.c        - link_flags persisted in the MIDI config section
#   schism/page_midi.c        - the two toggles and the live status readout
#
# WATCH: link_flags link_init link_quit link_apply_flags link_poll link_audio_begin
#        link_audio_end link_num_peers link_session_tempo abl_link_create
#        abl_link_capture_audio_session_state abl_link_audio_sink_create
# =============================================================================

Feature: Sharing tempo, transport and audio over Ableton Link
  As someone playing schism alongside other music software,
  I want it to share a beat, a transport and its audio over the network,
  So that it belongs in a setup with Live and everything else that speaks Link.

  @shipped @build-verified @hw-untested
  Scenario: Two switches on the MIDI page, off until asked
    # cite: schism/page_midi.c -- widgets 17 and 18, rows 38 and 39
    # cite: schism/midi-core.c -- link_flags in the [MIDI] config section, default 0
    Given a build with Link compiled in
    Then the MIDI page has an "Ableton Link" toggle and a "Link Audio out" toggle
    And both are off until switched on, and the setting persists
    And turning them on or off takes effect immediately, with no restart

  @shipped @build-verified @hw-untested
  Scenario: The page says what Link is actually doing
    # cite: schism/page_midi.c -- peer count and session tempo drawn at row 38
    # Without this the only evidence was in the message log, which is a poor way to
    # answer "is it working?".
    Given Link is switched on
    Then the page shows the peer count and the session tempo
    And it says "off", or "not in this build", when that is the case

  @shipped @build-verified @hw-untested
  Scenario: Tempo and transport follow the session
    # cite: schism/link.c link_audio_begin -- abl_link_capture_audio_session_state
    #       then abl_link_tempo, applied via song_set_current_tempo
    # cite: schism/link.c link_poll -- song_start / song_stop on the APP thread
    Given Link is switched on and another peer changes the tempo
    Then schism's tempo follows it
    And starting or stopping playback on either side starts or stops the other

  @shipped @build-verified @hw-verified
  Scenario: Link Audio publishes schism's output as a channel
    # cite: schism/link.c link_audio_end -- retain_buffer / memcpy / commit
    # cite: abl_link_audio_sink_create(link, "Schism Tracker", 65536)
    Given "Link Audio out" is switched on
    When schism renders a buffer
    Then it is committed to a Link Audio sink named "Schism Tracker"
    And other Link Audio software on the network can listen to it
    # VERIFIED 2026-08-19 from the installed app bundle, launched the way Spotlight
    # launches it, with an independent Link Audio receiver on the network:
    #     channels seen: 1
    #        [0] name="Schism Tracker" peer="Schism Tracker"
    #     peers=1

  @lib-verified
  Scenario: The vendored library really does form a session
    # Not just "it links" -- two abl_link instances in one process, built with the
    # exact flags configure.ac uses, one setting 138 BPM:
    #     PEERS=1 TEMPO=138.00 BEAT=0.234  -> session works
    # So the submodule, the include paths, the platform define and the C++17 flag
    # are all correct, independently of schism.
    Given the library is built the way configure.ac builds it
    Then two peers discover each other and share a tempo and a beat

  @design-note
  Scenario: Exactly one C++ translation unit, and schism stays C
    # Link is C++17 header-only; schism is C99 with no C++ anywhere. The bridge is
    # Link's OWN C API (link/extensions/abl_link), so the only file compiled as C++
    # is abl_link.cpp -- about five seconds, one object. Every schism source that
    # touches Link includes link.h and calls C functions.
    Given a C project and a C++ library
    Then the library's own C wrapper is the seam, and it is one file wide

  @design-note
  Scenario: Which thread is allowed to do what
    # abl_link_capture_audio_session_state is documented realtime-safe, so tempo
    # follow and the audio commit happen in the callback. song_start() and
    # song_stop() are nowhere near realtime-safe, so transport sync does NOT: the
    # audio thread only records what Link wants (g_want_playing) and link_poll(),
    # called from the event loop beside check_update(), acts on it.
    # g_told_playing stops us echoing our own announcement back at the session.
    Given a realtime callback and a transport that allocates
    Then the callback notices and the app thread acts

  @design-note
  Scenario: Off by default at BOTH levels, and why
    # --enable-link defaults to no: Link needs a C++17 compiler and the submodule,
    # and only supports Windows, macOS and Linux -- while sys/ still has wii, wiiu,
    # xbox, os2 and dos, which must keep building. A default build compiles
    # schism/link.c to stubs and contains ZERO abl_link symbols (verified with nm).
    # And even in a Link build it is off until switched on: joining a network
    # session and announcing an audio channel should never happen by surprise.
    Given a tree that also targets consoles and DOS
    Then the feature is opt-in to compile and opt-in to run

  @corrected
  Scenario: Enabling Link Audio crashed, twice over, and both were threading
    # Esa switched "Link Audio out" on and schism said "crashed". Two races, both mine:
    #
    # 1. ONE session-state object shared by both threads. abl_link_session_state is
    #    documented "Thread-safe: no", and link_audio_begin (audio callback) was
    #    capturing into the same object link_set_tempo / link_set_playing use from the
    #    UI. There are two now, g_state_audio and g_state_app, one per thread. This
    #    stayed hidden while only tempo follow was on, because nothing on the app
    #    thread touched the state in that configuration.
    #
    # 2. The sink was created and destroyed from the UI thread while the audio
    #    callback was using it -- and the sink calls are "Thread-safe: no" too. Every
    #    mutation in link_apply_flags and link_quit now happens with song_lock_audio()
    #    held, so the callback cannot be inside link_audio_begin/end at the time. The
    #    log_appendf messages are deferred until after the unlock, because holding the
    #    audio lock across UI drawing is how you get something that sounds like a
    #    dropout.
    #
    # Verified after the fix: the app stays up, and a receiver sees the channel.
    Given a library that marks its calls "Thread-safe: no"
    Then the UI must not create, destroy or share them behind the audio callback

  @corrected
  Scenario: link.c must be compiled even when Link is not
    # It was first listed inside "if USE_LINK" in Makefile.am. But the call sites in
    # audio_playback.c, main.c, midi-core.c and page_midi.c are unconditional, so a
    # default build would have failed at link time with undefined link_* symbols.
    # It is always compiled now and self-stubs behind #ifndef USE_LINK. Caught by
    # actually running a default build rather than assuming one.
    Given a feature reached from unconditional call sites
    Then its stubs are part of every build

  @corrected
  Scenario: AC_PROG_CXX cannot be called conditionally
    # Putting it inside "if test x$enable_link = xyes" made configure die with
    # 'conditional "am__fastdepCXX" was never defined' -- automake derives that from
    # AC_PROG_CXX and needs it at top level whenever any C++ source appears in
    # Makefile.am. It is unconditional now; it only DETECTS a compiler, and nothing
    # C++ is compiled without --enable-link.
    Given automake needs the C++ dependency machinery declared once, at top level
    Then the compiler check is unconditional and only its USE is conditional

  # --- open items -----------------------------------------------------------

  @todo
  Scenario: Tempo resolution -- the known limit
    # IT's tempo is an INTEGER BPM in 31..255, so a Link session at 120.5 cannot be
    # matched: link_audio_begin rounds, and the residual is a slow phase drift
    # against the session. Nothing here is broken; it is a format limit.
    # The fix, when phase lock is attempted, is to stop rounding the tempo and
    # instead nudge the samples-per-tick -- csf->tempo_factor (player/csndfile.c:47)
    # is the obvious lever, since it already scales tick length.
    Given IT tempo is an integer and Link's is not
    Then following it exactly needs sub-BPM tick control, not a rounder tempo

  @todo
  Scenario: Bar/phase lock -- the part that would make it feel like Link
    # Tempo follow keeps the SPEED right; it does not make schism start on the
    # downbeat with everyone else. That needs IT rows mapped onto Link beats
    # (abl_link_beat_at_time / abl_link_phase_at_time, already captured per buffer as
    # g_beat_at_buffer) and the tick scheduler nudged to hold phase -- a PLL in the
    # most timing-sensitive code in the player. Deliberately not attempted yet.
    Given the same tempo but a different phase
    Then two peers still do not sound locked, and this is the remaining work

  @todo
  Scenario: Receiving Link Audio, not just sending
    # The C API has sources as well as sinks (abl_link_audio_source_*), so listening
    # to another peer's channel is possible. Sending was done first because it is
    # what makes schism immediately useful to a Live user.
    Given the API supports sources
    Then receiving is a later, separate piece of work

  @shipped @build-verified @hw-verified
  Scenario: Live records real audio from schism
    # VERIFIED 2026-08-19/20 by Esa: Live 12.4.3, Link Audio on, "Schism Tracker"
    # selected as the track's Audio From, monitor In -- recorded a real waveform.
    #
    # Two things were needed, and both are worth knowing:
    #   - Live's Link Audio LATENCY had to be raised from its default. Esa: "by
    #     increasing the latency i got the audio recording working." Raise it until
    #     the recording is clean; it is the receive buffer.
    #   - request_max_num_samples has to be called BEFORE retain_buffer, not only
    #     after a valid-but-small handle -- see the @corrected scenario.
    Given Live has Link Audio on and a track set to the Schism Tracker channel
    When schism plays
    Then Live records its audio

  @design-note
  Scenario: "Sync to Incoming Audio" is Live's setting, not ours
    # Live's own tooltip: "If Link Audio is enabled, setting the Sync to Incoming
    # Audio to On delays Live's Link session by the set latency. This is useful when
    # monitoring audio from Link peers through Live. Note that the peers must have
    # Sync to Incoming Audio set to Off."
    # schism is a pure SENDER, so it is already in the state Live requires. There is
    # nothing to add here -- and adding it would be wrong.
    Given the receiving end delays its session to absorb peer latency
    Then the sending peer must not do the same, and schism does not

  @corrected
  Scenario: The channel was announced but permanently silent
    # request_max_num_samples was only reachable AFTER retain_buffer returned a valid
    # handle that happened to be too small. But before anything is listening the sink
    # hands out NO buffer at all (verified in isolation: valid=0, samples=NULL,
    # max=0), so the early return meant the sink was never asked for one -- the
    # channel appeared on the network and stayed mute forever.
    # The request happens first now, whenever the frame size changes.
    Given a sink that grants no buffer until it knows the size you want
    Then ask before you take, not after you fail

  @corrected
  Scenario: The status line kept landing on top of something
    # Placed at row 38 first: "Embed MIDI data" is drawn on that row from col 37,
    # AFTER it, so it read "0 peers, 1Embed". Moved to rows 39/40 beside the toggle
    # box, where it was cramped and clipped. It is one line at row 47 now, full
    # width: nothing is drawn below row 41 except the two buttons (rows 41-43,
    # 44-46). Third time, after actually enumerating what occupies each row.
    Given a text-mode page with no layout manager
    Then check every draw call on the row before claiming the space

  @todo
  Scenario: Non-16-bit output publication is untested on hardware
    # cite: schism/link.c link_to_s16 -- 8-bit (unsigned, centred and scaled),
    #       24-bit (top 16 of three LE bytes), 32-bit int, 32/64-bit float (clamped)
    # All of it converts into a static scratch buffer, so the callback never
    # allocates. Only the 16-bit path is confirmed against Live so far; the others
    # are read-verified conversions and want a listen at each depth.
    Given schism can output 8, 16, 24 or 32 bit, integer or float
    Then all of them convert to the int16 the sink wants
