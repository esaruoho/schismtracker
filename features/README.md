# schismtracker — Feature Reference

> **Generated** from the Gherkin report cards in this folder by `python3 print-card.py --readme`. Do not hand-edit — edit the `.feature` card and regenerate. Each entry below = one card: *what it does* (intent + behaviour scenarios) and *how it does it* (the procs/files the behaviour is cited to).

Each card is a triad: the `.feature` spec, a `.session.md` (the conversation that produced it), and a RESULT-LOG of what shipped.

## Contents

- [Sharing tempo, transport and audio over Ableton Link](#ableton-link) — `ableton-link.feature`
- [Fast sample loading into the pattern editor](#fast-sample-load) — `fast-sample-load.feature`


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

