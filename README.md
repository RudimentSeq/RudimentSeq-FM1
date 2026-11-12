# RudimentSeq for the M-VAVE FM-1

[![License: GPL-3.0-only](https://img.shields.io/badge/license-GPL--3.0--only-blue.svg)](LICENSE)

**The first groovebox firmware with drum rudiments built in.** RudimentSeq is a modified
[Felucca](https://github.com/hugelton/Felucca), Leo Kuroshita's open firmware for the M-VAVE FM-1.
Everything Felucca does is still here. On top of it, every track can play a rudiment instead of a
step pattern: the FM-1 becomes a rudiment groovebox with up to four rudiment lanes.

RudimentSeq began as a web app ([rudimentseq.com](https://rudimentseq.com)). This is its brain
rewritten in C for a 30-dollar synth.

## What a rudiment lane does

Turn **RUDI** on for a track (SEQ, second page) and the track stops reading its 16 steps. At every
step it plays the next hit of the chosen rudiment instead: right hand on one note, left hand on
another, with the accents, ghosts and grace notes of the real sticking.

- **174 rudiments**: the 40 PAS standard rudiments and the extended set of the web app, from a
  single stroke roll to flam paradiddle-diddles, in 16ths or triplets.
- **Hand notes** (NTE R / NTE L): any two notes of the DRUM kit, or of any engine, or two notes on
  external gear over USB MIDI.
- **Hand probability** (PRB R / PRB L): thin out one hand to open up the groove.
- **SMART**: a 128-step cycle of scenes that raises and lowers the density on its own.
- **FTHR**: feathering, the ghost notes get quieter the higher it goes.
- **Four lanes**: each of the four tracks has its own rudiment, notes and settings. Two lanes on two
  kits, a paradiddle on a bass line, a roll on the hats: rudiments are a rhythm distributor, not only
  a drum exercise.

Rudiment settings save with the project and come back at power-on.

## MIDI out (new in this firmware)

Felucca transmits the panel keys over USB MIDI. RudimentSeq also transmits what the sequencer,
the rudiment lanes and the arpeggiator play, on each track's channel (ROUTE), and never leaves a
note hanging on external gear (preset changes, engine changes and MIDI panic all send note-offs).
HOME > CLOCK > **SYNC** sends MIDI clock, Start and Stop at 24 ppqn when the FM-1 is the clock
master. The FM-1's TRS jack is MIDI in only; MIDI out is over USB.

## Install

The FM-1 connects by USB; no extra hardware. Installing any firmware is at your own risk.

- Web installer: the fork of Felucca's installer page (coming with the first release).
- From this tree, after [building](BUILDING.md): `python3 tools/fm1_install.py build/felucca.fwsc`
  (the package is named after upstream; the device reports itself as **RudimentSeq** on the MIDI
  bus and as FM-1_900 to installers, like Felucca).
- Going back: the installer's **Return to official V15**, or M-VAVE's own updater (FM1.dmg, stock
  V16). RudimentSeq keeps the stock OTA loader, as Felucca does.

Felucca's installer and web editor work with a RudimentSeq device: they accept both names.

## Status

r0.1, built on Felucca v1.9.0. Tested on one FM-1 (hardware revision with the UC25HQ80 flash).
The host test suite runs green but for one upstream text-spacing check. Please report problems in
the issues of this repository (https://github.com/RudimentSeq/RudimentSeq-FM1), not to Felucca's author. Contact: jem@rudimentseq.com.

Not yet here: a factory demo project that boots into two rudiment lanes; the rudiment list on the
web editor's pages. See [CHANGES-RUDIMENTSEQ.md](CHANGES-RUDIMENTSEQ.md) for what changed and when.

## Build

[BUILDING.md](BUILDING.md), unchanged from upstream. On a Mac the asset generators want Python 3.12:
`PYTHON=python3.12 bash build.sh`. The product string of the USB port is a build setting:
`FELUCCA_USB_PRODUCT=Felucca` builds a device that calls itself Felucca.

## Credits and licence

RudimentSeq is a modified Felucca. Felucca is Leo Kuroshita's
([@kurogedelic](https://github.com/kurogedelic), [Hügelton Instruments](https://hugelton.com));
its own README is kept at [docs/FELUCCA-README.md](docs/FELUCCA-README.md), its credits in
[CREDITS.md](CREDITS.md). The rudiment layer (`firmware/src/rudi.c`, `rudi_data.h`) and the
other changes are listed in [CHANGES-RUDIMENTSEQ.md](CHANGES-RUDIMENTSEQ.md).

GPL-3.0-only, like Felucca: the full source of this firmware is this repository. See
[LICENSE](LICENSE) and [LICENSING.md](LICENSING.md). NO WARRANTY.
