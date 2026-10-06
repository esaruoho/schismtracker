# Global channel insertion session

The user requested that insertion refuse when channel 64 contains content, telling them to fix it first. Implemented a whole-song preflight under the audio lock and rotation of the empty final channel to the cursor. Shift-Alt-Insert follows existing Shift-Alt channel movement conventions. Alt-Insert keeps row insertion.

Tests cover all six cell fields, rejection without dirtying or moving columns, successful shifts over variable-length patterns, settings preservation and invalid indices. App interaction remains untested. Initial build hit stale Automake 1.18 tooling; installed version is 1.19.

## How to get back

Transcript: file:///Users/esaruoho/.codex/sessions/2026/10/06/rollout-2026-10-06T10-15-44-01a11011-7fb5-7cd2-a03b-54cb44e0d3ff.jsonl
Session ID: 01a11011-7fb5-7cd2-a03b-54cb44e0d3ff
Resume: codex --resume 01a11011-7fb5-7cd2-a03b-54cb44e0d3ff
Verified session start: 2026-10-06T07:15:44.738Z
Bundled snapshot: [raw](global-channel-insert.transcript.jsonl), [readable](global-channel-insert.transcript.md).

Validation: build passed with AUTOMAKE=automake ACLOCAL=aclocal. The new insertion regression and existing exchange regression passed with DYLD_LIBRARY_PATH=/opt/homebrew/lib. make check runs no tests in this configuration. Full suite: 307 passed, 0 failed. Mac app bundle rebuilt with ./build_mac.sh --no-run; app interaction untested.
