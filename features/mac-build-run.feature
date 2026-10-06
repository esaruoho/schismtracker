# WHAT THIS CARD SPAWNS:
# codespace: build_mac.sh, local sys/macosx/Schism_Tracker.app packaging
# thinkspace: mac-build-run.session.md
# areaspace: owns local build, SDL runtime packaging and launch; excludes installation and source changes
# SESSION: mac-build-run.session.md
# RESULT: Local working tree; no commit or PR. Files: build_mac.sh and this card/session/transcript.
# WATCH: build_mac package_mac
# RESULT-LOG >>
#   2026-10-06  direct-commit  touched: build_mac package_mac
Feature: Build and launch Schism Tracker on macOS
  @build-verified
  Scenario: Build and package the configured checkout
    # cite: build_mac.sh build_mac and package_mac
    Given macOS with Homebrew SDL libraries and the build dependencies installed
    When build_mac.sh runs
    Then make builds the current checkout
    And the app contains the matching executable and SDL runtime libraries

  @build-verified
  Scenario: Repeat packaging of read-only Homebrew libraries
    # cite: build_mac.sh package_mac
    Given runtime libraries were previously copied into Resources
    When packaging runs again
    Then their owner write permission is restored before replacement
    And libSDL3.dylib resolves to the bundled SDL3 runtime

  @runtime-verified
  Scenario: Launch or activate the app
    # cite: build_mac.sh open
    Given the local bundle was packaged successfully
    When the script completes
    Then macOS receives an open request for the bundle
