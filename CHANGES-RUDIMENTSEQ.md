# RudimentSeq: changes to Felucca

This tree is a modified version of Felucca (https://github.com/hugelton/Felucca), GPL-3.0-only,
Copyright (C) 2026 Leo Kuroshita. The modifications are Copyright (C) 2026 RudimentSeq
(rudimentseq.com), released under the same licence, GPL-3.0-only. This file is the notice of
modification that section 5(a) of the licence asks for; the files changed carry a one-line
notice in their header as well.

Upstream base: Felucca commit 7435517 (2026-11-03, "fix(flash): recognize UC25HQ80 in newer
FM-1 units"), version v1.9.0.

## 2026-11-07 .. 2026-11-09: the rudiment layer

- New `firmware/src/rudi.c`, `firmware/src/rudi_data.h` (174 rudiments, generated from the
  RudimentSeq web app), `tests/rudi_test.c`.
- `core.h`: eight track parameters P_RUDION, P_RUDI, P_RUDPR, P_RUDPL, P_RUDSMA, P_RUDFEA,
  P_RUDNR, P_RUDNL (ids 106..113; P_E0 moved to 114, P_COUNT to 122).
- `params.c`: their descriptors; the SEQ page group gained the RUDI and RUDI 2 pages.
- `seq.c`: a track in rudi mode plays the rudiment's next hit at each step instead of its pattern.
- `project.c`: the rudiment settings persist with the project; `ui.c`, `ui_graph.c`, `ui_help.c`:
  the pages, the help lines and the step display in rudi mode.
- Tests: the suite's page, persistence, help and parameter-count pins updated; rudi_test added.

## 2026-11-09: MIDI out from the sequencer, rudiments, arpeggiator; SYNC

- `seq.c`: seq_note_on / seq_note_off send USB MIDI for sequencer, rudiment and arpeggio notes on
  the track's channel. With ARP on, the arpeggio is transmitted, not the held keys. Note-offs are
  sent on preset or engine change (arp_forget), MIDI All Sound Off / All Notes Off
  (`midi_control.c`), and the MIDI-in overflow path.
- `params.c`: the unused global G_SYNC became HOME > CLOCK > SYNC (OFF/ON): MIDI clock at 24 ppqn,
  Start and Stop, when the internal clock runs.
- `tests/scale_test.c`, `tests/ui_test.c`, `tests/rudi_test.c`, `tests/run_tests.sh`: the new truths.

## 2026-11-10: the web editor and its test; the fork's identity

- `web/editor.html`: the mock device knows the rudiment parameters and SYNC; the port filter
  accepts a device named RudimentSeq; page titles. `web/test_web.mjs`: the layout constants,
  a path fix for checkouts with spaces.
- `core.h`: RUDISEQ_NAME, RUDISEQ_VERSION, RUDISEQ_AUTHOR, RUDISEQ_URL. `ui_draw.c` (splash),
  `ui_menu.c` (about page and credits), `console.c` (banner), `editor.c` (INFO string): the fork
  names itself. `tools/build.py`: the USB product string, i.e. the MIDI port name, is RudimentSeq
  (FELUCCA_USB_PRODUCT in the environment overrides it). `tools/fm1_install.py`, `web/fm1ota.js`:
  the port filter accepts both names. `web/index_pkg.html`: titles.
- `README.md` is RudimentSeq's; Felucca's moved to `docs/FELUCCA-README.md`. `CREDITS.md`,
  `LICENSING.md`: this notice.

Unchanged: the update loader, the hardware layer (`firmware/bsp/`, MPL-2.0), the engines
(`firmware/engines/`, MIT), the package identity FM-1_900 and the USB ids, so Felucca's installer,
M-VAVE's updater and the recovery paths all still apply.

## 2026-11-19: STOP TO SAVE on the card (r0.1.2)

- `ui.c` act_stop, `ui_draw.c`: when the picked action is a save and the transport runs, the card
  says STOP (accent) in place of a dark OCT+ keycap, and the footer's hint row reads
  "[PLAY] STOP TO SAVE", lit; both go back when the transport stops. The 2 s message stays.
  (Jem saved nothing for a day because the dark keycap said nothing.)
- `core.h`: RUDISEQ_VERSION r0.1.2 (ABOUT, splash, console, editor INFO).
- `tests/ui_test.c`: the card's text while playing and after the stop, PROJECT and USER pages.
- Built and flashed as one package (FM1-flashes flash 15, md5 028d78bc), tried on the FM-1, released as
  r0.1.2 with those exact bytes.

## 2026-11-19: release r0.1.1

- The three 2026-11-18 sections below, built and flashed as one package (FM1-flashes flash 14,
  md5 19de5c94), run on the FM-1 for a day and released as r0.1.1 with those exact bytes. The
  firmware's own version string (RUDISEQ_VERSION, MENU > SYSTEM > ABOUT) still reads r0.1 in
  this build; it is bumped with the next one rather than reflashing an untested image.
- `docs/QUICK-GUIDE.md`: the RUDI 3 page and the SEQ page cycle; `README.md`: status r0.1.1.

## 2026-11-18: RUDI 3, random notes on the track's scale

- Three track parameters after NTE L: `RNG R`, `RNG L` (0..24 semitones; 114, 115) and `CHG` (%, 116; default 100):
  with a range, the hand plays a random note of the track's scale (SCL: ROOT, SCALE; CHR = chromatic) between its
  NTE and NTE + RNG, drawn anew on CHG % of its hits and kept on the rest (`rudi.c` rudi_note, scale_has; `seq.c`
  declares scale_mask ahead of the include). The engine ids move from 114..121 to 117..124: P_E0 117, P_COUNT 125.
  Projects and user sounds of before load by count as always (the new three take their defaults, motion ids move
  with P_E0); MOTION_MAX 114 -> 112 makes room in the fixed project store (4 bytes spare). A new SEQ page, RUDI 3
  (K4 empty); `ui_help.c`; `web/editor.html` (its table, P_COUNT, P_E0); the tests' pins; `tests/rudi_test.c`: T4.

## 2026-11-18: RUDI 1X .. 4X, the lane's rate

- `params.c`: RUDI (K1 of the RUDI page) is OFF, 1X, 2X, 3X or 4X instead of OFF / ON: the lane steps every 1..4
  steps of its track (`rudi.c` rudi_step: skip; the hit holds through the skipped steps). 3X on a 1/16 track is the
  dotted-eighth feel the track's DIV cannot give. Stored values 0 / 1 mean what they did. `ui_graph.c`: the rate in
  the card's header; `ui_help.c`; `web/editor.html`: the editor's list. `tests/rudi_test.c`: T3 at 3X.

## 2026-11-18: a saved sound carries its rudiment

- `upreset.c` (up_load): a user sound saved while RUDI was on brings the eight lane values (RUDI, RUDMT, NTE R / L,
  PRB R / L, SMART, FTHR) with it. Before, the lane was `param_kept` on every load, so SAVE on a paradiddle bass
  stored the lane (the record always held it) but loading the sound left the track's lane as it was: on a fresh
  track or after power-on the sound came back without its rudiment. A sound saved with RUDI off, and every Felucca
  record, still leaves the lane alone. `ui.c`: SAVE held (undo) swaps the lane back too (undo.rudi).
- `tests/rudi_save_test.c`: the project and user-sound round trips of the lane, the undo, STOP TO SAVE.

## 2026-11-12: the splash

- `firmware/src/ui_draw.c` draw_splash: a new power-on screen in the web app's colours (a paradiddle in the two hands'
  colours, the name in the large face, the version, the upstream and its maker), not the saved palette;
  `tools/gen_aa_font.py` L_CHARS: the large face gains the name's eight lower-case glyphs.
- `firmware/src/gfx.c` glyph: a lower-case letter a face has is drawn as itself; the capital stands in only for one it
  lacks (the large face had none; it keeps mapping the rest).
- `firmware/src/main.c`: the splash stays RUDISEQ_SPLASH_MS (2.5 s, was 0.4 s; `firmware/src/core.h`); a key or
  button after 0.5 s ends it, and is waited out before the UI reads it.
- `platforms/host/felucca_host.c`, `platforms/sdl/felucca_sdl.c`, `platforms/libretro/lr_check.c`,
  `platforms/web/emu_test.mjs`, `tests/ui_render.c`: follow the new length and layout; the GREY / MONO pixel lint
  skips the splash (its colours are fixed in every palette).

## 2026-11-12: CI

- `platforms/web/emu_test.mjs`: the controller-knob checks boot their devices past the 2.5 s splash (they turned the
  knob under it).
- `firmware/src/params.c`, `firmware/src/ui_draw.c`: a rudiment's name is free text to the layout lint (fmt_free), so
  a long one may ellipsise on its card, plain and MENU > LARGE, like a preset name. No change on the screen.

