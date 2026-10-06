# macOS build and launch session

## How to get back

- Transcript: file:///Users/esaruoho/.codex/sessions/2026/10/06/rollout-2026-10-06T10-08-39-01a1100b-02f3-75f3-9de9-2ad6e6c2e541.jsonl
- Session ID: 01a1100b-02f3-75f3-9de9-2ad6e6c2e541
- Resume: `codex --resume 01a1100b-02f3-75f3-9de9-2ad6e6c2e541`
- Session began 2026-10-06 10:08:39 local time, from transcript filename. Bundled transcript contains exact event timestamps.

Esa requested a macOS build and launch. Compilation succeeded, but the initial bundle lacked SDL libraries. The assistant incorrectly described a process blocked in a fatal-error alert as successfully launched. The user supplied the crash text and then a screenshot of the SDL3 loading error. SDL2 here is sdl2-compat and requires libSDL3.dylib beside it, in addition to the versioned filename used by Schism's loader.

Esa then requested build_mac.sh to build and run. The script uses existing configuration, bootstraps configuration when absent, copies Homebrew SDL runtime libraries, supplies the SDL3 alias, and opens the local app. Read-only copied libraries initially prevented repeated packaging; chmod before replacement corrected this. One complete build/package/launch returned zero and its process remained running. A second open -n failed to spawn another instance, so launch changed to plain open, which can activate an existing instance. No application source was modified, installed to /Applications, committed, or pushed.

Esa confirmed the app worked and requested pushing everything to esaruoho/schismtracker, adding schism and schismtracker aliases, and building macOS, Linux and Windows release assets. Another session/user committed the initial scripts and channel exchange as 48cf360d while this session worked. This session added an idempotent alias installer and source-distribution entries. The installer successfully added its source line to ~/.bash_profile. GitHub authentication initially failed in the sandbox but succeeded outside it using macOS Keychain; no renewed login was necessary. Subsequent rebuilds encountered concurrent edits from the channel-insertion session, so release validation must follow the final committed state.

The shared checkout subsequently committed and pushed those changes and channel insertion in d34bbe03. Regenerating autotools and rebuilding against that state succeeded; the test suite reported 307 passed and zero failed. Alias installation was tested twice against a temporary profile and produced one source line; a real Bash login shell resolved both requested commands. Release publication uses the existing tag-triggered macOS, Ubuntu and MinGW workflows.
