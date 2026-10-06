# WHAT THIS CARD SPAWNS:
# codespace: .github/workflows/mingw.yml release-upload step
# thinkspace: windows-release-upload.session.md
# areaspace: owns Windows release repository selection; excludes compilation and platforms
# SESSION: windows-release-upload.session.md
# RESULT: Upload fix pending commit. 20261006-esa Windows x64, ARM64 and ARMv7 artifacts recovered from Actions run 37430021200 and attached to the release. Workflow YAML verified; next tagged run still untested.
# WATCH: GH_REPO
# RESULT-LOG >>
#   2026-10-06  direct-commit  touched: GH_REPO
Feature: Upload Windows binaries from a nested checkout
  @built @runtime-untested
  Scenario: Release upload selects the fork outside the checkout
    # cite: .github/workflows/mingw.yml Attach to GitHub release, GH_REPO
    Given Windows packages are created outside the nested schism checkout
    When gh release create or gh release upload runs
    Then GH_REPO selects github.repository explicitly
    And git repository discovery is unnecessary

  @build-verified
  Scenario: Recover this release's completed Windows builds
    # cite: .github/workflows/mingw.yml Upload artifact
    Given run 37430021200 compiled and uploaded Windows artifacts for tag 20261006-esa
    When those artifacts are downloaded and packaged with their runtime DLLs and license files
    Then each archive contains the executable and all nine expected package files
    And the archive integrity checks pass
    And the x64, ARM64 and ARMv7 packages are attached to the release
