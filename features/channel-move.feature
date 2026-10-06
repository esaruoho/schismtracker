# WHAT THIS CARD SPAWNS:
# codespace: schism/mplink.c song_exchange_channels; schism/page_patedit.c pattern_move_channel
# thinkspace: channel-move.session.md
# areaspace: owns channel exchanges and editor shortcuts; excludes mixer changes
# SESSION: channel-move.session.md
# RESULT: Feature delivered directly to master in 48cf360d; card authored in d34bbe03. No PR. Files: include/song.h, include/test-funcs.h, schism/mplink.c, schism/page_patedit.c, test/cases/mplink.c.
# WATCH: song_exchange_channels pattern_move_channel shift_gesture test_song_exchange_channels
# RESULT-LOG >>
#   2026-10-06  direct-commit  touched: song_exchange_channels
Feature: Move a channel across every pattern
  @build-verified
  Scenario: Exchange and restore two channels throughout a song
    # cite: schism/mplink.c song_exchange_channels; test/cases/mplink.c test_song_exchange_channels
    Given patterns of different lengths and distinct channel data and panning
    When two channels are exchanged
    Then their pattern columns and channel settings trade places
    And exchanging again restores them
    And identical and out-of-range indices leave them unchanged

  @built @runtime-untested
  Scenario: Move a channel with the pattern editor shortcuts
    # cite: schism/page_patedit.c pattern_move_channel and pattern_editor_handle_alt_key
    Given the pattern editor has a current channel
    When Shift Alt Left or Shift Alt Right is pressed
    Then the channel trades places with its neighbor across the song
    And the cursor and multichannel-record flag follow it
    And no selection begins for the gesture
