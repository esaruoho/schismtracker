# Fast Sample Load Session

## How To Get Back

- Transcript: `file:///Users/esaruoho/.codex/sessions/2026/09/23/rollout-2026-09-23T19-30-34-01a0cf1a-c92e-7c31-8bed-24d3370e8041.jsonl`
- Bundled transcript: `features/fast-sample-load.transcript.jsonl`
- Readable transcript: `features/fast-sample-load.transcript.md`
- Session ID: `01a0cf1a-c92e-7c31-8bed-24d3370e8041`
- Resume: `codex --resume 01a0cf1a-c92e-7c31-8bed-24d3370e8041`
- Session started: 2026-09-23 19:30:34 EEST
- Session note written: 2026-09-23 19:33:52 EEST
- Identification: found by fixed-string match on `scroll-lock method of loading a sample` and `capslock is a two-way-door`; not guessed.

## User Request

Esa asked to port the recent impulse-tracker workflow into Schism Tracker:

- Scroll Lock while in Load Sample should load the selected sample into a previously free sample slot and generate a matching instrument in the corresponding free instrument slot.
- It should immediately switch to the pattern editor and enable pattern follow mode.
- Caps Lock should be a two-way door: from anywhere it opens Load Sample; pressed again in Load Sample it accepts the selected sample, switches to the pattern editor, and disables pattern follow.
- The intent is fast blind sample browsing into immediate tracking.

## Implementation Notes

The implementation keeps the existing loader path as the authority for actually reading samples. The new exported helper, `sample_load_current_file_to_free_slot(int follow)`, chooses the first index where both the sample and instrument are empty, sets the current sample/instrument to that index, sets the requested follow state, and calls the normal selected-file load logic.

`finish_load()` now remembers whether the load came from the fast path. Normal Enter loading still prompts/returns as before. Fast loading creates the host instrument directly and jumps to the requested page after load completion. For stereo samples, the destination and host-creation intent survive the stereo conversion dialog.

The global key path owns Caps Lock because it must work from any page. The global Scroll Lock path now lets Load Sample consume plain Scroll Lock before the existing playback-tracing toggle; outside Load Sample, the old Scroll Lock behavior remains.

After runtime testing, Caps Lock needed an SDL3/macOS event fix: SDL3 extended-key events were being dropped before page routing, so the SDL3 event backend now preserves physical Caps Lock by scancode. The installed app bundle also needed SDL3 dylib aliases matching Schism's dynamic loader probes.

Stereo fast-loading now treats "Both" as implicit for the fast path. Normal sample loading still shows the Left/Both/Right dialog, but Caps Lock and Scroll Lock fast-loads keep stereo intact and continue straight to the pattern editor.

Right Arrow in the Load Sample file list now opens the current sample-browser directory in the native file manager. The page calls `os_open_folder()` on `samp_cwd`, macOS maps that to `NSWorkspace openURL`, Windows maps it to `ShellExecute`, and Shift-Right quicksave gestures keep their previous behavior.

## Verification

- Ran `make -j4`.
- Ran `make check`.
- Build and test harness completed successfully.
- Rebuilt after the stereo fast-load change and re-ran `make -j4`, `make check`, and `git diff --check`.
- Rebuilt after the Right Arrow native-folder-open change and re-ran `make -j4`, `make check`, and `git diff --check`.
- Installed the rebuilt binary into `/Applications/SchismTracker.app/Contents/MacOS/schismtracker`.
- Verified the installed binary points at `@executable_path/../Resources/libutf8proc.3.dylib`.
- Verified bundled SDL3 aliases exist in `/Applications/SchismTracker.app/Contents/Resources/`.
- Existing compiler warnings appeared in unrelated files: integer-literal warning in `schism/disko.c`, deprecation warnings in `sys/macosx/osdefs.m`, and linker warning for missing `/usr/local/opt/zlib/lib`.
- Runtime UI key-path testing is not done in this session, so the card marks scenarios as `@runtime-untested`.
