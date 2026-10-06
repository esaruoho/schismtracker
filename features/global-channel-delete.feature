# WHAT THIS CARD SPAWNS:
# codespace: schism/mplink.c, schism/page_patedit.c, include/song.h, test/cases/mplink.c
# thinkspace: global-channel-delete.session.md
# areaspace: Owns global channel removal; inverse of global-channel-insert. Preserves
#   row deletion (Alt-Delete) and channel movement.
# SESSION: global-channel-delete.session.md
# RESULT: Working tree only; no commit or PR. Sources above plus include/test-funcs.h.
# WATCH: song_remove_channel song_channel_is_empty song_clear_channel_locked pattern_remove_channel
# RESULT-LOG >>
#   2026-10-06  direct-commit  touched: song_remove_channel song_channel_is_empty song_clear_channel_locked pattern_remove_channel
#   2026-10-06  working-tree  added: song_remove_channel song_channel_is_empty pattern_remove_channel

Feature: Safely delete a channel throughout the song
  @build-verified @runtime-untested
  Scenario: Remove an empty channel without asking
    # cite: schism/page_patedit.c pattern_editor_handle_alt_key; schism/mplink.c song_remove_channel
    Given the current channel has no note data in any pattern
    When Shift-Alt-Delete is pressed in the pattern editor
    Then that channel is removed from every pattern
    And higher channels shift left with their settings and mute states
    And channel 64 becomes blank with default settings
    And the song is marked as needing save

  @build-verified @runtime-untested
  Scenario: Confirm before removing a channel that holds data
    # cite: schism/page_patedit.c pattern_remove_channel_confirm, song_channel_is_empty
    Given the current channel has note data in some pattern
    When Shift-Alt-Delete is pressed in the pattern editor
    Then a dialog asks whether to delete the channel in all patterns
    And nothing changes unless the user confirms

  @stock
  Scenario: Alt-Delete still deletes a pattern row
    # cite: schism/page_patedit.c pattern_editor_handle_alt_key
    When Alt-Delete is pressed without Shift
    Then a row is deleted in the current pattern

  @sim-verified
  Scenario: Automated removal regression
    # cite: test/cases/mplink.c test_song_remove_channel
    # NOTE: schismtrackertest hangs on macOS (pre-existing, see harness.c --paper comment),
    #   so this was run by linking the real mplink.o/csndfile.o into a standalone driver.
    Given variable-length allocated patterns
    When the regression exercises removal, the empty-channel predicate, and an insert/remove round trip
    Then every assertion passes
