# Windows release upload repair

## How to get back

- Session ID: 01a1100b-02f3-75f3-9de9-2ad6e6c2e541
- Transcript: file:///Users/esaruoho/.codex/sessions/2026/10/06/rollout-2026-10-06T10-08-39-01a1100b-02f3-75f3-9de9-2ad6e6c2e541.jsonl
- Resume: `codex --resume 01a1100b-02f3-75f3-9de9-2ad6e6c2e541`
- Session start: 2026-10-06T07:08:39Z; exact transcript timestamps bundled beside this card.

Esa requested macOS, Linux and Windows packages on the fork's releases page. Tag 20261006-esa triggered the existing workflows. Linux uploaded successfully. The Windows ARMv7 compiler and artifact upload succeeded, but release attachment failed at 2026-10-06T07:35:09Z with `fatal: not a git repository`. Workflow checkout lives under schism while package/upload commands run in its parent. Setting GH_REPO to github.repository fixes repository selection for future releases. Recovery of the existing tag's already-built packages uses downloaded Actions artifacts, without changing release source or moving its tag. The changed workflow itself has not yet run on a new tag.

All three Windows build jobs (x64, ARM64, ARMv7) failed only at release attachment. Their uploaded Actions artifacts were downloaded, zipped with the same nine required files, checked with zip integrity verification, and uploaded to release 20261006-esa. Release API verification confirmed all three Windows assets and the Linux asset. Ruby YAML parsing confirmed the corrected upload step sets GH_REPO to github.repository. macOS was still compiling dependencies at this point.
