# Schism Tracker

Schism Tracker is a free and open-source reimplementation of [Impulse
Tracker](https://github.com/schismtracker/schismtracker/wiki/Impulse-Tracker),
a program used to create high quality music without the requirements of
specialized, expensive equipment, and with a unique "finger feel" that is
difficult to replicate in part. The player is based on a highly modified
version of the [Modplug](https://openmpt.org/legacy_software) engine, with a
number of bugfixes and changes to [improve IT
playback](https://github.com/schismtracker/schismtracker/wiki/Player-abuse-tests).

Where Impulse Tracker was limited to i386-based systems running MS-DOS, Schism
Tracker runs on almost any platform that [SDL 2](https://www.libsdl.org/index.php) 
supports. Currently builds are provided for Linux, Mac OS X, and Windows. Most 
development is currently done on 64-bit Linux. Schism will most likely build on
_any_ architecture supported by GCC4 (e.g. alpha, m68k, arm, etc.) but it will 
probably not be as well-optimized on many systems.

See [the wiki](https://github.com/schismtracker/schismtracker/wiki) for more
information.

![screenshot](http://schismtracker.org/screenie.png)

## Download

The latest stable builds for Windows, macOS, and Linux are available from [the
releases page](https://github.com/schismtracker/schismtracker/releases). Builds
can also be installed from some distro repositories on Linux, but these
versions may not have the latest bug fixes and enhancements. Older builds for
other platforms can be found on
[the wiki](https://github.com/schismtracker/schismtracker/wiki). Installing via
Homebrew on macOS is no longer recommended, as the formula for Schism Tracker
is not supported or maintained by anyone directly involved in the project.

## Compilation

See the
[docs/](https://github.com/schismtracker/schismtracker/tree/master/docs) folder
for platform-specific instructions.

```sh
./configure && make
```

A plain `git clone` is enough for that. This fork carries the Ableton Link sources
as a git submodule, but nothing needs it unless you ask for Link — see below.

### Ableton Link + Link Audio (this fork)

Off by default, at two levels: it is not compiled in unless you ask, and even then it
is switched off until you turn it on in the program.

```sh
git clone --recurse-submodules https://github.com/esaruoho/schismtracker.git
# or, in a clone you already have:
git submodule update --init --recursive

./configure --enable-link
make
```

`--enable-link` needs a C++17 compiler and the `link/` submodule. Link supports
Windows, macOS and Linux only, which is why it is opt-in: the wii, wiiu, xbox, os2
and dos targets have to keep building, and a default build contains no Link code at
all.

Once built in, turn it on in the program on the **MIDI page (Shift-F1)**:

- **Ableton Link** — join the session: follow its tempo, and share transport
  start/stop in both directions.
- **Link Audio out** — additionally publish this instance's output as a Link Audio
  channel named "Schism Tracker", which other Link Audio software (e.g. Ableton Live
  12.4+) can listen to and record.

The bottom of that page reports the peer count, the session tempo, and whether audio
is actually being sent, so you can see it working. Both switches persist in the
config as `link_flags` under `[MIDI]`.

Notes from getting it working against Live 12.4.3:

- In Live: Settings → Link → **Audio: On**, and set the track's **Audio From** to
  the "Schism Tracker" channel with **Monitor: In**.
- If the recording comes out silent, **raise Live's Link Audio latency** — the
  default receive buffer can be too tight.
- Live's "Sync to Incoming Audio" belongs on the *receiving* side; Live's own tooltip
  says peers must have it **Off**, and this fork is a pure sender, so there is
  nothing to set here.
- Ableton Link is dual-licensed GPLv2+ / proprietary, so it is compatible with
  Schism's GPLv2.

What works: tempo follow, transport both ways, and audio into Live. What does not yet:
bar/phase lock (the tempo matches, but schism is not put on the downbeat with its
peers), and receiving Link Audio rather than only sending. See
[features/ableton-link.feature](features/ableton-link.feature).

## Packaging status

[![Packaging status](https://repology.org/badge/vertical-allrepos/schismtracker.svg)](https://repology.org/project/schismtracker/versions)
