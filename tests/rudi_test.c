/* SPDX-License-Identifier: GPL-3.0-only
 * rudi mode (firmware/src/rudi.c): track 1 plays a Single Paradiddle on the
 * DRUM kit (snare R / rim L), track 2 a Single Stroke Roll on the hats with
 * Smart Prob. 8 bars at the power-on tempo are rendered to a WAV; then the
 * sticking of track 1's first two bars is checked against R L R R L R L L,
 * track 1 fires every step (prob 100), track 2 fires fewer (the scenes thin
 * it) on both hands, and the audio is not silent. MIDI OUT (1.8): every hit
 * track 1 fired left as a note-on on its channel, snare (NTE R) for R and rim
 * (NTE L) for L in the paradiddle's order, and by the STOP every note-on had
 * its note-off, so nothing hangs on external gear. SYNC ON (CLOCK's fourth
 * knob, OFF by default): one Start before the first note, MIDI clock at 24
 * ppqn (6 pulses a 1/16 step), one Stop with the STOP and no clock after it.
 *   cc -O2 -w -DRUDI_STATS -Ibuild/gen -Ifirmware/src -o build/host/rudi_test tests/rudi_test.c -lm
 *   build/host/rudi_test build/rudi_demo.wav */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define RUDI_STATS 1
#include "../platforms/host/firmware.c"
#include "../platforms/host/tracks.c"

static void wav_hdr(FILE *f, uint32_t frames)
{
    uint32_t v;
    uint16_t a = 1, ch = 2, ba = 4, bits = 16;
    uint32_t sr = FS, br = FS * 4;
    fwrite("RIFF", 1, 4, f); v = 36 + frames * 4; fwrite(&v, 4, 1, f);
    fwrite("WAVEfmt ", 1, 8, f); v = 16; fwrite(&v, 4, 1, f);
    fwrite(&a, 2, 1, f); fwrite(&ch, 2, 1, f); fwrite(&sr, 4, 1, f); fwrite(&br, 4, 1, f);
    fwrite(&ba, 2, 1, f); fwrite(&bits, 2, 1, f);
    fwrite("data", 1, 4, f); v = frames * 4; fwrite(&v, 4, 1, f);
}

/* MIDI OUT as the USB host would see it: drain the out ring (64 packets) and keep track 1's note-ons and -offs, and
 * SYNC's realtime: Start (before any note?), clock pulses (after the Stop?), Stop */
static uint8_t mo_note[64];
static uint32_t mo_on, mo_off, rt_start, rt_clock, rt_stop, rt_late, start_first;
static void drain_midi_out(void)
{
    while (mo_r != mo_w) {
        uint32_t pk = midi_out_q[mo_r % MQ], st = (pk >> 8) & 0xFFu;
        mo_r++;
        if ((pk & 0x0Fu) == 0x0Fu) {                /* realtime: CIN 0xF, the status alone */
            if (st == 0xFAu) {
                rt_start++;
                if (!mo_on)
                    start_first = 1;
            } else if (st == 0xF8u) {
                rt_clock++;
                if (rt_stop)
                    rt_late++;
            } else if (st == 0xFCu)
                rt_stop++;
            continue;
        }
        if ((st & 0x0Fu) != trk_midi_ch(0))
            continue;                               /* track 1's channel only */
        if ((st & 0xF0u) == 0x90u) {
            if (mo_on < 64u)
                mo_note[mo_on] = (uint8_t)((pk >> 16) & 127u);
            mo_on++;
        } else if ((st & 0xF0u) == 0x80u)
            mo_off++;
    }
}

int main(int argc, char **argv)
{
    const char *out = argc > 1 ? argv[1] : "build/rudi_demo.wav";
    static const uint8_t PARA[16] = {0, 1, 0, 0, 1, 0, 1, 1,    /* R L R R L R L L x 2 */
                                     0, 1, 0, 0, 1, 0, 1, 1};
    uint32_t f, i, period, bar, frames, fails = 0;
    int64_t energy = 0;
    FILE *w;

    host_tracks_init();
    host_drums(&trk[0]);                    /* both rudi lanes on the DRUM kit */
    host_drums(&trk[1]);
    trk[2].p[P_LEVEL] = 0;                  /* the other tracks silent */
    trk[3].p[P_LEVEL] = 0;
    trk[0].p[P_RUDION] = 1;                 /* T1: the defaults (#16 Single Paradiddle, snare / rim) */
    trk[1].p[P_RUDION] = 1;                 /* T2: Single Stroke Roll on the hats, Smart Prob */
    trk[1].p[P_RUDI] = 0;
    trk[1].p[P_RUDSMA] = 1;
    trk[1].p[P_RUDNR] = 42;
    trk[1].p[P_RUDNL] = 46;

    period = div_samples((uint32_t)trk[0].p[P_SDIV]);
    bar = period * 16u;
    frames = bar * 8u;                      /* 8 bars */
    w = fopen(out, "wb");
    if (!w) { printf("rudi: cannot write %s\n", out); return 1; }
    wav_hdr(w, frames);

    usb.config = 1;                         /* a USB host is listening: MIDI OUT packets are queued */
    mo_w = mo_r = 0;
    if (GP[G_SYNC].def != 0 || GP[G_SYNC].max != 1 || song.g[G_SYNC] != 0 || song.g[G_CLOCK] != 0) {
        printf("rudi: FAIL SYNC should be an OFF / ON global, OFF by default, with CLK INT\n"); fails++;
    }
    song.g[G_SYNC] = 1;                     /* SYNC ON: Start, 24 ppqn clock and Stop go to MIDI OUT too */
    transport_req = 1;
    for (f = 0; f < frames; f += CTL) {
        int32_t o[2 * CTL];
        mix_block(o, CTL);
        drain_midi_out();
        for (i = 0; i < CTL; i++) {
            int16_t s[2] = {(int16_t)(o[2 * i] > 32767 ? 32767 : o[2 * i] < -32768 ? -32768 : o[2 * i]),
                            (int16_t)(o[2 * i + 1] > 32767 ? 32767 : o[2 * i + 1] < -32768 ? -32768 : o[2 * i + 1])};
            fwrite(s, 2, 2, w);
            energy += (int64_t)s[0] * s[0];
        }
    }
    fclose(w);
    seq_stop();                             /* STOP: the hit still sounding gets its note-off, on MIDI OUT too */
    drain_midi_out();

    /* track 1: every step fired (prob 100, no rests in a paradiddle), sticking right */
    printf("rudi: T1 steps %u fired R %u L %u   T2 steps %u fired R %u L %u\n",
           rudi_stat_steps[0], rudi_stat_fired[0][0], rudi_stat_fired[0][1],
           rudi_stat_steps[1], rudi_stat_fired[1][0], rudi_stat_fired[1][1]);
    if (rudi_stat_steps[0] < 120u || rudi_stat_fired[0][0] + rudi_stat_fired[0][1] != rudi_stat_steps[0]) {
        printf("rudi: FAIL T1 should fire every step\n"); fails++;
    }
    if (rudi_stat_fired[0][0] != rudi_stat_fired[0][1]) {
        printf("rudi: FAIL a paradiddle is half R half L\n"); fails++;
    }
    for (i = 0; i < 16u && i < rudi_stat_n; i++)
        if (rudi_stat_hand[i] != PARA[i]) {
            printf("rudi: FAIL sticking at hit %u: %u (want %u)\n", i, rudi_stat_hand[i], PARA[i]); fails++;
            break;
        }
    /* track 2: Smart Prob thins it, both hands still speak */
    if (rudi_stat_fired[1][0] + rudi_stat_fired[1][1] >= rudi_stat_steps[1]) {
        printf("rudi: FAIL T2 Smart Prob should drop some hits\n"); fails++;
    }
    if (!rudi_stat_fired[1][0] || !rudi_stat_fired[1][1]) {
        printf("rudi: FAIL T2 both hands should fire\n"); fails++;
    }
    if (energy / (int64_t)frames < 1000) {
        printf("rudi: FAIL render is (near) silent\n"); fails++;
    }
    /* MIDI OUT (1.8): every hit track 1 fired left the box as a note-on on its channel, R the snare (NTE R 38) and
     * L the rim (NTE L 37) in the paradiddle's order; by the STOP every note-on had its note-off */
    printf("rudi: MIDI OUT T1 note-ons %u note-offs %u (fired %u)\n", mo_on, mo_off,
           rudi_stat_fired[0][0] + rudi_stat_fired[0][1]);
    if (mo_on != rudi_stat_fired[0][0] + rudi_stat_fired[0][1]) {
        printf("rudi: FAIL every fired hit should reach MIDI OUT\n"); fails++;
    }
    for (i = 0; i < 16u && i < mo_on; i++)
        if (mo_note[i] != (PARA[i] ? 37u : 38u)) {
            printf("rudi: FAIL MIDI OUT note at hit %u: %u (want %u)\n", i, mo_note[i], PARA[i] ? 37u : 38u); fails++;
            break;
        }
    if (mo_on != mo_off) {
        printf("rudi: FAIL MIDI OUT note-ons and note-offs should balance after STOP (a note hangs)\n"); fails++;
    }
    /* SYNC ON (1.8): one Start before the first note, 24 ppqn = 6 clock pulses a 1/16 step (one more when the render
     * ends exactly on a step boundary: that step's pulse 0), one Stop with the STOP and no clock after it */
    printf("rudi: SYNC Start %u (first %s) clock %u (steps %u x 6) Stop %u, clock after Stop %u\n",
           rt_start, start_first ? "yes" : "NO", rt_clock, rudi_stat_steps[0], rt_stop, rt_late);
    if (rt_start != 1u || !start_first) {
        printf("rudi: FAIL SYNC should send one Start, before the first note\n"); fails++;
    }
    if (rt_clock < 6u * rudi_stat_steps[0] || rt_clock > 6u * rudi_stat_steps[0] + 1u) {
        printf("rudi: FAIL SYNC clock should run at 24 ppqn (6 pulses a 1/16 step)\n"); fails++;
    }
    if (rt_stop != 1u || rt_late) {
        printf("rudi: FAIL SYNC should send one Stop with the STOP and no clock after it\n"); fails++;
    }
    printf("rudi: sticking R L R R L R L L %s, MIDI OUT %s, %s (%u bars to %s)\n",
           fails ? "BROKEN" : "ok", mo_on && mo_on == mo_off ? "balanced" : "BROKEN",
           fails ? "FAILED" : "all checks passed", 8u, out);
    return fails ? 1 : 0;
}
