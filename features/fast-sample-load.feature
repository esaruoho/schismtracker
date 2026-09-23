# =============================================================================
# WIKI PAGE / REPORT CARD: Fast Sample Load Keys
#
# WHAT THIS CARD SPAWNS:
#   codespace  - schism/page.c global key routing, schism/page_loadsample.c
#                sample-loader accept path, and include/it.h exported hook.
#   thinkspace - fast-sample-load.session.md records the request and decisions.
#   areaspace  - OWNS: Caps Lock / Scroll Lock sample-load accelerators.
#                MUST NOT TOUCH: normal Enter loading, existing Scroll Lock
#                tracing outside Load Sample, and sample-library bulk loading.
#
# Report-card legend:
#   @shipped          - in this working tree
#   @build-verified   - `make -j4` and `make check` completed after the change
#   @app-launch-verified - installed macOS app process stays alive after launch
#   @runtime-untested    - not yet driven manually in the tracker UI
#   @stock            - pre-existing behavior kept as a boundary
#
# Innards linked back to this card:
#   schism/page.c - capslock_sample_load_check opens/accepts the loader; global
#                   Scroll Lock dispatch reaches the loader before tracing toggle.
#   schism/page_loadsample.c - sample_load_current_file_to_free_slot chooses a
#                              blank sample/instrument pair and completes load;
#                              Right Arrow opens the current sample folder.
#   include/osdefs.h - os_open_folder abstraction for native file managers.
#   sys/win32/osdefs.c - Explorer folder opening through ShellExecute.
#   sys/sdl3/events.c - preserves physical Caps Lock scancodes from SDL3
#                       extended key events so the global key path can see them,
#                       and bypasses text composition for Caps Lock.
#   sys/macosx/macosx-sdlmain.m - disables macOS press-and-hold character
#                                 picking for the app process.
#   sys/macosx/Schism_Tracker.app/Contents/Info.plist - disables
#                                                       ApplePressAndHoldEnabled.
#   include/it.h - exported loader accelerator prototype for the global key path.
#   /Applications/SchismTracker.app/Contents/Resources/libSDL3-3.0.0.dylib
#       - symlink to bundled libSDL3.0.dylib for Schism's dynamic SDL3 probe.
#
# Commit log:   working tree, not committed
# SESSION:      fast-sample-load.session.md
# RESULT:       Feature delivery pending commit; direct working-tree change, no PR
#
# WATCH: capslock_sample_load_check sample_load_current_file_to_free_slot
#        first_free_sample_instrument_pair fastload_destination_page
#        fastload_create_host SCHISM_KEYSYM_SCROLLLOCK
#        CHN_STEREO libSDL3-3.0.0.dylib
#        ApplePressAndHoldEnabled
#        os_open_folder open_current_sample_folder
#
# RESULT-LOG >> (auto-maintained by the report-card hooks - newest below)
#   2026-09-23  direct-commit  touched: capslock_sample_load_check sample_load_current_file_to_free_slot
# =============================================================================

Feature: Fast sample loading into the pattern editor
  As a tracker user browsing samples,
  I want Caps Lock and Scroll Lock to load the selected sample into a fresh slot
  and jump straight to the pattern editor,
  So that choosing a sound and immediately tracking it is one gesture.

  @shipped @build-verified @runtime-untested
  Scenario: Caps Lock is a two-way door into sample picking and non-follow tracking
    # cite: sys/sdl3/events.c sdl3_event() - preserves Caps Lock by scancode
    #       instead of dropping SDL3 extended-mask key events on macOS.
    # cite: schism/page.c capslock_sample_load_check (~line 495) - opens Load Sample
    #       from any page, and accepts the selection when already there.
    # cite: schism/page_loadsample.c sample_load_current_file_to_free_slot (~line 794)
    #       sets playback_tracing and midi_playback_tracing to 0 before loading.
    Given no dialog is open
    When Caps Lock is pressed outside Load Sample
    Then Schism switches to the Load Sample page
    When Caps Lock is pressed again on a sample row
    Then that sample loads and Schism switches to the pattern editor
    And pattern follow is off

  @shipped @build-verified @runtime-untested
  Scenario: Scroll Lock accepts a sample and lands in pattern follow
    # cite: schism/page.c handle_key_global (~line 1102) - Load Sample gets first
    #       claim on plain Scroll Lock before the normal tracing toggle.
    # cite: schism/page_loadsample.c sample_load_current_file_to_free_slot (~line 794)
    #       sets playback_tracing and midi_playback_tracing from the follow flag.
    Given the Load Sample page is focused on a sample row
    When Scroll Lock is pressed
    Then the selected sample loads and Schism switches to the pattern editor
    And pattern follow is on

  @shipped @build-verified @runtime-untested
  Scenario: Fast loading picks a matching free sample and instrument slot
    # cite: schism/page_loadsample.c first_free_sample_instrument_pair (~line 793)
    #       requires both current_song->samples[n] and instruments[n] to be empty.
    # cite: schism/page_loadsample.c finish_load (~line 535) - creates the host
    #       instrument and jumps to the requested page after normal load completion.
    Given sample slot 04 and instrument slot 04 are both empty
    And slots 01 through 03 are not a free pair
    When either fast-load key accepts a selected sample
    Then the sample is loaded into sample slot 04
    And a host instrument is generated in instrument slot 04
    And both current sample and current instrument point at slot 04

  @stock
  Scenario: Existing Scroll Lock behavior survives outside Load Sample
    # cite: schism/page.c handle_key_global (~line 1102) - non-loader Scroll Lock
    #       still toggles playback tracing, and Alt-Scroll Lock still toggles MIDI.
    Given the current page is not Load Sample
    When Scroll Lock is pressed
    Then Schism keeps the existing playback-tracing toggle behavior
    And Alt-Scroll Lock keeps the existing MIDI input toggle behavior

  @shipped @build-verified @runtime-untested
  Scenario: Fast loading a stereo sample keeps both channels without prompting
    # cite: schism/page_loadsample.c finish_load (~line 551) - fast-load host
    #       creation bypasses the stereo conversion dialog and preserves stereo.
    Given the selected sample is stereo
    When Caps Lock or Scroll Lock fast-loads it
    Then Schism keeps both channels
    And the Left/Both/Right dialog does not interrupt tracking

  @shipped @app-launch-verified
  Scenario: Installed macOS app can load bundled SDL3
    # cite: schism/loadso.c loadso_libtool_fmts - macOS loader probes
    #       @executable_path/../Resources/libSDL3-3.0.0.dylib for SDL3-3.0.
    Given SchismTracker.app is installed in /Applications
    When the app launches
    Then the bundled SDL3 library is found by the runtime loader
    And the schismtracker process remains alive instead of showing the fatal SDL3 dialog

  @shipped @build-verified
  Scenario: Caps Lock does not trigger macOS character-picking UI
    # cite: sys/macosx/macosx-sdlmain.m main - sets ApplePressAndHoldEnabled
    #       false for the app process before NSApplication startup.
    # cite: sys/sdl3/events.c sdl3_pump_events - Caps Lock keydown bypasses
    #       SDL3 composition/pending-text handling.
    # cite: sys/macosx/Schism_Tracker.app/Contents/Info.plist - persists
    #       ApplePressAndHoldEnabled=false in the bundle template.
    Given SchismTracker is focused on macOS
    When Caps Lock is used as the sample-load command
    Then macOS press-and-hold character picking stays disabled
    And Caps Lock is handled as a tracker command instead of text input

  @shipped @build-verified @runtime-untested
  Scenario: Right Arrow opens the current sample folder in the native file manager
    # cite: schism/page_loadsample.c file_list_handle_key - plain Right Arrow
    #       opens samp_cwd and does not repeat-spam file manager windows.
    # cite: include/osdefs.h os_open_folder - routes native folder opening.
    # cite: sys/macosx/osdefs.m macosx_open_folder - macOS uses NSWorkspace.
    # cite: sys/win32/osdefs.c win32_open_folder - Windows uses ShellExecute.
    Given the Load Sample file list is focused
    When Right Arrow is pressed without modifiers
    Then the current sample-browser folder opens in Finder, Explorer, or the platform file manager
    And Shift-Right quicksave gestures remain unchanged
