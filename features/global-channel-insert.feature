# WHAT THIS CARD SPAWNS:
# codespace: schism/mplink.c, schism/page_patedit.c, include/song.h, test/cases/mplink.c
# thinkspace: global-channel-insert.session.md
# areaspace: Owns global insertion; preserves row insertion and channel movement.
# SESSION: global-channel-insert.session.md
# RESULT: Working tree only; no commit or PR. Sources above, include/test-funcs.h,
# helptext/pattern-editor, card, session, transcripts and generated feature views.
# WATCH: song_insert_channel song_exchange_channels_locked pattern_editor_handle_alt_key
# RESULT-LOG >>
#   2026-10-06  direct-commit  touched: song_insert_channel song_exchange_channels_locked

Feature: Safely insert a channel throughout the song
  @build-verified @runtime-untested
  Scenario: Refuse insertion when channel 64 contains data
    # cite: schism/mplink.c song_insert_channel; test/cases/mplink.c test_song_insert_channel
    Given any allocated pattern has a nonempty cell on channel 64
    When global channel insertion is requested
    Then no pattern or channel settings change
    And the editor tells the user to move or clear channel 64 first

  @build-verified @runtime-untested
  Scenario: Insert an empty channel at the cursor
    # cite: schism/page_patedit.c pattern_editor_handle_alt_key; schism/mplink.c song_insert_channel
    Given channel 64 is empty across every allocated pattern
    When Shift-Alt-Insert is pressed in the pattern editor
    Then an empty channel appears at the cursor in every pattern
    And subsequent channels shift right with their settings and mute states
    And channel 64 settings and recording toggle rotate into the empty channel
    And the song is marked as needing save

  @stock
  Scenario: Alt-Insert still inserts a pattern row
    # cite: schism/page_patedit.c pattern_editor_handle_alt_key
    When Alt-Insert is pressed without Shift
    Then a row is inserted in the current pattern

  @sim-verified
  Scenario: Automated insertion regression
    # cite: test/cases/mplink.c test_song_insert_channel
    Given variable-length allocated patterns
    When the regression test exercises all six cell fields and successful insertion
    Then every assertion passes with DYLD_LIBRARY_PATH=/opt/homebrew/lib ./schismtrackertest
