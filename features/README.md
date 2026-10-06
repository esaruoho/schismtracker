# schismtracker — Feature Reference

> **Generated** from the Gherkin report cards in this folder by `python3 print-card.py --readme`. Do not hand-edit — edit the `.feature` card and regenerate. Each entry below = one card: *what it does* (intent + behaviour scenarios) and *how it does it* (the procs/files the behaviour is cited to).

Each card is a triad: the `.feature` spec, a `.session.md` (the conversation that produced it), and a RESULT-LOG of what shipped.

## Contents

- [Sharing tempo, transport and audio over Ableton Link](#ableton-link) — `ableton-link.feature`
- [Move a channel across every pattern](#channel-move) — `channel-move.feature`
- [Fast sample loading into the pattern editor](#fast-sample-load) — `fast-sample-load.feature`
- [Safely insert a channel throughout the song](#global-channel-insert) — `global-channel-insert.feature`
- [Build and launch Schism Tracker on macOS](#mac-build-run) — `mac-build-run.feature`
- [Upload Windows binaries from a nested checkout](#windows-release-upload) — `windows-release-upload.feature`


<a id="ableton-link"></a>
## Sharing tempo, transport and audio over Ableton Link

`features/ableton-link.feature`

**What it does:** As someone playing schism alongside other music software, I want it to share a beat, a transport and its audio over the network, So that it belongs in a setup with Live and everything else that speaks Link.

**Behaviour (19 scenarios):**

- Two switches on the MIDI page, off until asked — `@shipped @build-verified @hw-untested`
- The page says what Link is actually doing — `@shipped @build-verified @hw-untested`
- Tempo and transport follow the session — `@shipped @build-verified @hw-untested`
- Link Audio publishes schism's output as a channel — `@shipped @build-verified @hw-verified`
- The vendored library really does form a session
- Exactly one C++ translation unit, and schism stays C
- Which thread is allowed to do what
- Off by default at BOTH levels, and why
- Enabling Link Audio crashed, twice over, and both were threading
- link.c must be compiled even when Link is not
- AC_PROG_CXX cannot be called conditionally
- Tempo resolution -- the known limit — `@todo`
- Bar/phase lock -- the part that would make it feel like Link — `@todo`
- Receiving Link Audio, not just sending — `@todo`
- Live records real audio from schism — `@shipped @build-verified @hw-verified`
- "Sync to Incoming Audio" is Live's setting, not ours
- The channel was announced but permanently silent
- The status line kept landing on top of something
- Non-16-bit output publication is untested on hardware — `@todo`

**How it does it:** **Key procs:** `link_flags`, `link_init`, `link_quit`, `link_apply_flags`, `link_poll`, `link_audio_begin` · **Source files:** `schism/page_midi.c`, `schism/midi-core.c`, `schism/link.c`

**Grade:** @build-verified ×5 · @hw-untested ×3 · @hw-verified ×2 · @shipped ×5 · @todo ×4


<a id="channel-move"></a>
## Move a channel across every pattern

`features/channel-move.feature` · [session](channel-move.session.md)

**Behaviour (2 scenarios):**

- Exchange and restore two channels throughout a song — `@build-verified`
- Move a channel with the pattern editor shortcuts — `@built @runtime-untested`

**How it does it:** **Key procs:** `song_exchange_channels`, `pattern_move_channel`, `shift_gesture`, `test_song_exchange_channels` · **Source files:** `schism/mplink.c`, `test/cases/mplink.c`, `schism/page_patedit.c`

**Grade:** @build-verified ×1 · @built ×1 · @runtime-untested ×1


<a id="fast-sample-load"></a>
## Fast sample loading into the pattern editor

`features/fast-sample-load.feature` · [session](fast-sample-load.session.md)

**What it does:** As a tracker user browsing samples, I want Caps Lock and Scroll Lock to load the selected sample into a fresh slot and jump straight to the pattern editor, So that choosing a sound and immediately tracking it is one gesture.

**Behaviour (8 scenarios):**

- Caps Lock is a two-way door into sample picking and non-follow tracking — `@shipped @build-verified @runtime-untested`
- Scroll Lock accepts a sample and lands in pattern follow — `@shipped @build-verified @runtime-untested`
- Fast loading picks a matching free sample and instrument slot — `@shipped @build-verified @runtime-untested`
- Existing Scroll Lock behavior survives outside Load Sample — `@stock`
- Fast loading a stereo sample keeps both channels without prompting — `@shipped @build-verified @runtime-untested`
- Installed macOS app can load bundled SDL3 — `@shipped`
- Caps Lock does not trigger macOS character-picking UI — `@shipped @build-verified`
- Right Arrow opens the current sample folder in the native file manager — `@shipped @build-verified @runtime-untested`

**How it does it:** **Key procs:** `capslock_sample_load_check`, `sample_load_current_file_to_free_slot` · **Source files:** `sys/sdl3/events.c`, `schism/page.c`, `schism/page_loadsample.c`, `schism/loadso.c`, `include/osdefs.h`, `sys/win32/osdefs.c`

**Grade:** @build-verified ×6 · @runtime-untested ×5 · @shipped ×7 · @stock ×1


<a id="global-channel-insert"></a>
## Safely insert a channel throughout the song

`features/global-channel-insert.feature` · [session](global-channel-insert.session.md)

**Behaviour (4 scenarios):**

- Refuse insertion when channel 64 contains data — `@build-verified @runtime-untested`
- Insert an empty channel at the cursor — `@build-verified @runtime-untested`
- Alt-Insert still inserts a pattern row — `@stock`
- Automated insertion regression — `@sim-verified`

**How it does it:** **Key procs:** `song_insert_channel`, `song_exchange_channels_locked`, `pattern_editor_handle_alt_key` · **Source files:** `schism/mplink.c`, `test/cases/mplink.c`, `schism/page_patedit.c`

**Grade:** @build-verified ×2 · @runtime-untested ×2 · @sim-verified ×1 · @stock ×1


<a id="mac-build-run"></a>
## Build and launch Schism Tracker on macOS

`features/mac-build-run.feature` · [session](mac-build-run.session.md)

**Behaviour (4 scenarios):**

- Build and package the configured checkout — `@build-verified`
- Repeat packaging of read-only Homebrew libraries — `@build-verified`
- Launch or activate the app — `@runtime-verified`
- Launch aliases point to this checkout — `@runtime-verified`

**How it does it:** **Key procs:** `build_mac`, `package_mac`, `schism_alias_command`, `source_line`

**Grade:** @build-verified ×2 · @runtime-verified ×2


<a id="windows-release-upload"></a>
## Upload Windows binaries from a nested checkout

`features/windows-release-upload.feature` · [session](windows-release-upload.session.md)

**Behaviour (2 scenarios):**

- Release upload selects the fork outside the checkout — `@built @runtime-untested`
- Recover this release's completed Windows builds — `@build-verified`

**How it does it:** **Key procs:** `GH_REPO`

**Grade:** @build-verified ×1 · @built ×1 · @runtime-untested ×1

