# RudimentSeq on the FM-1: quick guide

Felucca v1.9.0 controls plus the RudimentSeq additions. Sources: Felucca's wiki (Controls, Menu, Features) and the firmware. r0.1.1, 2026-11-19.

![FM-1 controls](controls.jpg)

(The controls diagram is Felucca's, by Leo Kuroshita. Its SEQ line predates 1.5: holding SEQ now opens SEQ TOOLS, the song is under GLO.)

## The panel

| Control | Does |
|---|---|
| **SELECT** | Tempo (BPM). MENU > BPM LOCK keeps it fixed; then GLO + SELECT |
| **MASTER** | Volume (speaker, phones; USB too unless MENU > USB LEVEL FIXED) |
| **ALGORITHM** | Selects the track T1 to T4, on every page |
| **PRESETS** | The track's sound, on every page that is not a list |
| **KNOB 1 to 4** | The four columns of the page shown |
| **PLAY** | Start / stop all four tracks (green while playing) |
| **REC** | Arms the track to record, on every page; stopped, it starts playing too |
| **OCT− / OCT+** | Octave down / up. In dialogs and the menu: OCT+ does it, OCT− goes back. Both together: octave reset |
| **Keys** | Play the selected track, F3 to G5. On SEQ > STEP they enter notes (drum track: white keys = steps, black keys = lanes) |

## Five rules

1. **Tap** a page button for its page, tap again for the next page of that family. HOME returns home. Buttons act when let go.
2. **Hold** FX, GLO, SCL, EDIT, REC or SEQ for its quick layer on the keys and knobs. Let go and it closes.
3. **Double-tap** a layer's button to keep its layer open without holding; tap it again to close.
4. **Hold SAVE** = undo the last load or edit (again: redo). **Hold HOME** = the menu.
5. **OCT+** confirms, **OCT−** goes back; in a quick layer OCT− puts things back as the layer found them.

## The pages, by button

| Button | Taps go round | Held |
|---|---|---|
| HOME | HOME, MIXER (LEVEL PAN REV MUTE), CLOCK (BPM, swing, clock source, **SYNC**) | the menu |
| SEQ | STEP, **RUDI**, **RUDI 2**, **RUDI 3**, DETAIL, AUTOMATION | SEQ TOOLS |
| GLO | SONG | GLO layer: mutes, solos, levels, tap tempo |
| SAVE | USER, PRESETS, PHRASES, PROJECT, TOOLS | undo |
| LFO | LFO, LFO 2, LFO DEST, MOD | MOMENTARY: hold LFO, turn a knob, let go and it snaps back (OCT+ while holding keeps it) |
| EDIT | EDIT 1, EDIT 2, VOICE, VOICE 2, VOICE 3 (more on FM6, DRUM, SLICE, ANALOG) | EDIT layer: engines |
| FX | FX, INSERT, INSERT 2, SLICER | FX layer: performance effects |
| SCL | SCL, CHORD | SCL layer: root and scale |
| ENV, ARP | their pages (ARP, ARP 2) | |
| REC | arms the track | REC layer |

## Quick layers: what the keys do while the button is held

**GLO held** (mixing)

| Keys | Does |
|---|---|
| F3 G3 A3 B3 (white) | SOLO T1 T2 T3 T4 while held |
| F#3 G#3 A#3 C#4 (black) | MUTE T1 T2 T3 T4, latched. Lit = sounding, dark = muted |
| C4 | UNMUTE ALL |
| D4 | MIDI LEARN (then move a controller CC, then a knob) |
| E4 | FILL while held (steps with a FILL condition play) |
| F4 | TAP tempo |
| KNOB 1 to 4 | T1 to T4 levels |
| GLO + SELECT | tempo (even with BPM LOCK). GLO + PLAY: restart from the top |

**FX held** (performance effects; momentary, MENU > FX LATCH makes them toggle)

| White key | F3 | G3 | A3 | B3 | C4 | D4 | E4 |
|---|---|---|---|---|---|---|---|
| Effect | repeat 1/8 | repeat 1/16 | repeat 1/32 | REVERSE | LPF | HPF | TAPE STOP |

| White key | F4 | G4 | A4 | B4 | C5 | D5 | E5 |
|---|---|---|---|---|---|---|---|
| Effect | FREEZE | OCT UP | OCT DN | FLANGER | PHASER | REV THROW | ECHO THROW |

Black keys F#3 G#3 A#3 C#4: momentary mutes T1 to T4. Hold a white key and turn PRESETS to give it another effect. FX + OCT−: everything off. KNOB 1 to 4: the effects' macros.

**SEQ held** (SEQ TOOLS, on the selected track; each tool is one undo, OCT− puts the pattern back)

White keys from F3, in order: CLEAR, REVERSE, SHIFT <, SHIFT >, RANDOM, COOK, BEAT (drum), EUCLID, then on a DRUM track the lane's CLR, REV, FILL, RND, then DOUBLE and HALF. KNOB 1 to 4: LEN, DIV, SWING, GATE.

**EDIT held** (the engine)

White keys from F3: the engines in order ANALOG, FM6, PHASE, LOFI, SAMPLE, VOICE, TRIO, WHEEL, GRAIN, PHYS, NOISE, SLICE, DRUM, then INIT and CLEAN INIT. On a DRUM track the black keys F#3 upwards mute the eight lanes (KICK first), latched.

**SCL held**: a key sets the root; KNOB 2 picks the scale (shown 4 × 4); the chord keys on the CHORD page.

**REC held**: F3 CLEAR the sequence; G3 CLICK, A3 COUNT-IN, B3 CLICK LEVEL (each press steps the setting; KNOB 1 to 3 do the same).

## Sequencer

- **SEQ > STEP**: the piano roll (drum: the grid). Keys enter notes at the cursor; KNOB 3 = note length. REC on STEP while stopped = step recording (MENU > CHORD ENTRY: HOLD plays a chord in, ADD adds one note at a time).
- **Live recording**: tap REC (arms the track, starts the transport if stopped), play. Overdub by leaving REC on. MENU > COUNT-IN gives one or two bars first.
- **Parameter lock**: hold a step and turn KNOB 1 to 4. EDIT with the step held clears its locks.
- **SEQ > DETAIL**: the cursor step's CHANCE (or condition: 1:4, FILL …), RATCH, NUDGE, VEL on the knobs.
- **SEQ > AUTOMATION**: everything on the track's steps as one list.

## RudimentSeq: the rudiment lanes

- **SEQ, tap twice** (STEP then RUDI). KNOB 1 **RUDI** OFF, 1X, 2X, 3X or 4X: the track stops reading its steps and plays the rudiment, a hit every 1 to 4 steps of the track (3X on 1/16 is the dotted-eighth lope; hold SEQ, KNOB 2 for the track's DIV). KNOB 2 **RUDMT** picks one of 174. KNOB 3 **NTE R** and KNOB 4 **NTE L**: the right and left hand's notes (any kit sound, any engine note, or external gear over USB MIDI).
- **Tap SEQ again** for RUDI 2: KNOB 1 **SMART** (a 128-step scene cycle that raises and lowers the density), KNOB 2 **FTHR** (feathering, the ghosts get quieter), KNOB 3 **PRB R**, KNOB 4 **PRB L** (hand probabilities, thin a hand out).
- **Tap SEQ again** for RUDI 3: KNOB 1 **RNG R** and KNOB 2 **RNG L**, a range in semitones (0 to 24) above the hand's note: the hand plays a random note of the track's scale in that range (SCL page: ROOT and SCALE, CHR for any note). KNOB 3 **CHG** is how often a hit picks a new note: 100 scatters, 20 drifts.
- Each of the four tracks has its own lane and settings. They save with the project and come back at power-on.
- **MIDI out** is automatic: the sequencer, the lanes and the arp go out over USB on each track's channel, and no note is ever left hanging.
- **HOME > CLOCK, KNOB 4 SYNC ON**: MIDI clock, Start and Stop out when the FM-1 is the clock master.
- **To hear only a rudiment lane**: hold GLO and mute the other tracks on the black keys (or solo its white key), or set MUTE on HOME > MIXER.

## Saving and loading

| Want | Do |
|---|---|
| Save a sound | stop, tap SAVE (USER), KNOB 1 picks a slot, OCT+, OCT+ again (name it with the keys). Saved with RUDI on, the sound carries its rudiment: load it anywhere and the lane comes back |
| Load a sound | PRESETS knob on any page, or SAVE > PRESETS to browse by category |
| Save / load a project | SAVE > PROJECT: KNOB 1 slot A to D, KNOB 3 LOAD, KNOB 4 SAVE (OCT+ confirms) |
| Start a new project | MENU > SYSTEM > CLEAR ALL, OCT+ twice (cannot be undone) |
| Power off | just switch off: MENU > RESTORE LAST brings the music back as you left it |

## The menu (hold HOME)

ALGORITHM moves between the tabs DISPLAY, CONTROL, AUDIO, MIDI, SYSTEM; PRESETS moves through the rows; any knob or OCT+ / OCT− changes the value; HOME closes it. Worth knowing: COLOR (palettes), LARGE (big knob cards), FX LATCH, BPM LOCK, CLICK and COUNT-IN, MIDI IN (CH1-4, SEL, TRACK …), USB SERIAL OFF on macOS 13 to 15 for USB audio in, HELP ON for a hint when a page opens, ABOUT (shows the RudimentSeq release).

## If something goes wrong

- No sound but the sequencer runs: check the mutes first, GLO held (black keys lit = sounding) and HOME > MIXER > MUTE.
- Stuck notes on external gear: change the preset or send MIDI panic; both send note-offs.
- Back to the official firmware: the browser installer at https://rudimentseq.github.io/RudimentSeq-FM1/ has "Return to official V15"; M-VAVE's own updater works too. The update loader is never touched.
- Source, releases, issues: https://github.com/RudimentSeq/RudimentSeq-FM1
