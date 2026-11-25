/* SPDX-License-Identifier: GPL-3.0-only
 * rudi.c — RudimentSeq's rudiment brain (rudimentseq.com) as a Felucca step
 * source. A track in rudi mode ignores its step pattern: each (swung) step
 * boundary plays the next hit of a rudiment instead — hand R or L on its own
 * note, with the web app's velocity model (accent 108, ghost/feather toward
 * 20, grace 30) and per-hand probability. "Smart Prob" runs a 128-step cycle
 * of 16 L/R scenes (solos, trades, builds, fades) scaling both hands.
 * Runs in the audio ISR: integer math only, ends in seq_note_on/off (seq.c:
 * the track's voice and MIDI OUT on its channel, as the keys and the step
 * sequencer; seq.c calls rudi_step where it would play a step).
 * Data: rudi_data.h (core.h includes it), 174 rudiments from the app.
 * State: the track's own parameters P_RUDION .. P_RUDNL (params.c): the
 * RUDI pages edit them, projects and user presets carry them. Here only
 * what runs: the hit cursor, the smart step, the held note, the dice. */

typedef struct {
    uint8_t hit;        /* next hit of the rudiment */
    uint8_t sstep;      /* Smart Prob step 0..127, advances every step, rests too */
    uint8_t lastrud;    /* the P_RUDI last played: a turn restarts the sticking */
    uint8_t sounding;   /* the note held until the next step + 1, 0: none */
    uint8_t skip;       /* track steps still to pass before the lane's next step (RUDI 2X..4X; 1X: always 0) */
    uint8_t cur[2];     /* RUDI 3: the hand's last random note + 1 (0: none yet); kept on 100 - CHG % of its hits */
    uint8_t pos;        /* PACK: the slot playing (CHAIN on: the lane moves to the next after REPS passes) */
    uint8_t pass;       /* .. passes of it done */
    uint32_t rng;
} rudi_lane_t;
static rudi_lane_t rudi[NTRK];
static uint8_t rudi_smart[2][128];   /* [hand R/L][step]: % of the knob, 16 scenes x 8 */
static uint8_t rudi_ready;

static void rudi_init(void)
{
    uint32_t i, k;
    for (i = 0; i < 128; i++) {                  /* the scenes, as the web app's SmartTable */
        uint32_t sc = i >> 3, s = i & 7, l = 100, r = 100;
        switch (sc) {
        case 1: l = 0; break;                                    /* R SOLO */
        case 2: if ((s >> 1) & 1) r = 15; else l = 15; break;    /* TRADE 2s */
        case 3: r = 0; break;                                    /* L SOLO */
        case 4: if (s & 1) r = 0; else l = 0; break;             /* ALTERNATE */
        case 5: l = 45; break;                                   /* R LEAD */
        case 6: if ((s & 3) == 3) l = r = 25; break;             /* GAPS */
        case 7: r = 45; break;                                   /* L LEAD */
        case 8: l = r = 50; break;                               /* SPARSE */
        case 9: l = 20; break;                                   /* R + L GHOST */
        case 10: l = r = 25 + s * 75 / 7; break;                 /* BUILD */
        case 11: r = 20; break;                                  /* L + R GHOST */
        case 12: if (s < 4) l = 10; else r = 10; break;          /* TRADE 4s */
        case 13: l = r = 35; break;                              /* THIN */
        case 14: l = 100 - s * 100 / 7; r = s * 100 / 7; break;  /* L -> R FADE */
        default: break;                                          /* 0, 15: FULL */
        }
        rudi_smart[0][i] = (uint8_t)r;
        rudi_smart[1][i] = (uint8_t)l;
    }
    for (k = 0; k < NTRK; k++)
        rudi[k].rng = 0x9e3779b9u + k * 0x85ebca6bu;
    rudi_ready = 1;
}

static uint32_t rudi_rand(rudi_lane_t *r)          /* xorshift32: the lane's own dice */
{
    r->rng ^= r->rng << 13; r->rng ^= r->rng >> 17; r->rng ^= r->rng << 5;
    return r->rng >> 16;
}
static uint32_t rudi_rand100(rudi_lane_t *r) { return rudi_rand(r) % 100u; }

static int scale_has(const track_t *t, int32_t n)  /* note n is on the track's scale (SCL page: ROOT, SCALE; CHR: any) */
{
    return (int)((scale_mask(t) >> (uint32_t)((n - t->p[P_ROOT] + 120) % 12)) & 1u);
}

/* RUDI 3 (2026-11-18): the hand's note. RNG 0: its NTE, as always. RNG 1..24: a note of the track's scale between
 * NTE and NTE + RNG, drawn at random on CHG % of the hand's hits and kept on the rest, so 100 scatters and 20 drifts */
static uint32_t rudi_note(track_t *t, rudi_lane_t *r, uint32_t hand)
{
    int32_t base = t->p[hand ? P_RUDNL : P_RUDNR] & 127, range = t->p[hand ? P_RUDRL : P_RUDRR], c[25];
    uint32_t cur = r->cur[hand], n = 0, k;
    if (range <= 0)
        return (uint32_t)base;
    if (cur && (int32_t)cur - 1 >= base && (int32_t)cur - 1 <= base + range && rudi_rand100(r) >= (uint32_t)t->p[P_RUDCHG])
        return cur - 1u;                           /* kept */
    for (k = 0; k <= (uint32_t)range && n < 25u; k++)   /* the scale's notes in the range */
        if (base + (int32_t)k <= 127 && scale_has(t, base + (int32_t)k))
            c[n++] = base + (int32_t)k;
    if (!n)
        return (uint32_t)base;
    k = (uint32_t)c[rudi_rand(r) % n];
    r->cur[hand] = (uint8_t)(k + 1u);
    return k;
}

/* the hit's chance: PRB of its hand, scaled by SMART's scene at smart step ss; 1 = it plays (the lane's dice) */
static int rudi_fires(const track_t *t, rudi_lane_t *r, const rudi_hit_t *h, uint32_t ss)
{
    uint32_t p = (uint32_t)(h->hand ? t->p[P_RUDPL] : t->p[P_RUDPR]);
    if (t->p[P_RUDSMA])
        p = p * rudi_smart[h->hand ? 1 : 0][ss] / 100u;
    return !(p < 100u && rudi_rand100(r) >= p);
}

/* the web app's velocity model (0..127): 90; an accent 108 (90 + (115 - 90) x 90 / 127); feathered toward 20; a
 * grace 30 */
static uint32_t rudi_vel(const track_t *t, const rudi_hit_t *h)
{
    uint32_t vel = 90, feather = (uint32_t)t->p[P_RUDFEA];
    if (h->accent)
        vel = 108;
    else if (feather)
        vel = 90u - (90u - 20u) * feather / 100u;
    if (h->grace)
        vel = 30;
    return vel;
}

static void rudi_hush(track_t *t)                  /* the held note off (stop, mode off) */
{
    rudi_lane_t *r = &rudi[trk_index(t)];
    if (r->sounding) {
        seq_note_off(t, r->sounding - 1u);         /* the voice and MIDI OUT (seq.c) */
        r->sounding = 0;
    }
}

/* PACK (2026-11-25): the track's pack of rudiments (track_t pack[npack]), walked when CHAIN (P_RUDCH) is on: after REPS
 * passes of the slot playing the next slot's rudiment becomes RUDMT (the sticking starts from its top, as a knob turn);
 * past the last slot the first again, and with RESHUF the pack is shuffled first, so no lap repeats the order. The
 * UI's own writes (the PACK page: fill, jump, shuffle, deal; ui_input.c pack_keys) are bytes, and the lane reads
 * pack[pos % npack] once a pass: nothing is half seen */
static void rudi_pack_shuffle(track_t *t, rudi_lane_t *r)   /* Fisher-Yates with the lane's dice */
{
    uint32_t i, n = t->npack;
    for (i = n; i > 1u; i--) {
        uint32_t j = rudi_rand(r) % i;
        uint8_t v = t->pack[i - 1u];
        t->pack[i - 1u] = t->pack[j];
        t->pack[j] = v;
    }
}
static void rudi_pack_take(track_t *t, rudi_lane_t *r, uint32_t slot)   /* slot's rudiment plays: RUDMT */
{
    uint32_t rud;
    if (!t->npack)
        return;
    r->pos = (uint8_t)(slot % t->npack);
    r->pass = 0;
    rud = (uint32_t)t->pack[r->pos] % NUM_RUDIMENTS;
    t->p[P_RUDI] = (int16_t)rud;
}
static void rudi_pack_step(track_t *t, rudi_lane_t *r)   /* a pass of the rudiment ended (its hit 0 is next) */
{
    if (!t->p[P_RUDCH] || !t->npack)
        return;
    if (++r->pass < (uint32_t)(t->p[P_RUDREP] > 0 ? t->p[P_RUDREP] : 1))
        return;
    r->pass = 0;
    r->pos = (uint8_t)((r->pos + 1u) % t->npack);
    if (r->pos == 0u && t->p[P_RUDCH] == 2)         /* RESHUF: a new order every lap */
        rudi_pack_shuffle(t, r);
    r->lastrud = (uint8_t)((uint32_t)t->pack[r->pos] % NUM_RUDIMENTS);   /* (hit is 0: no restart needed) */
    t->p[P_RUDI] = (int16_t)r->lastrud;
}
/* the UI (main loop): the slot playing, 0xFF when CHAIN is off or the pack is empty */
static uint32_t rudi_pack_pos(const track_t *t)
{
    const rudi_lane_t *r = &rudi[trk_index(t)];
    return t->p[P_RUDCH] && t->npack ? r->pos % t->npack : 0xFFu;
}
static void rudi_pack_jump(track_t *t, uint32_t slot) { rudi_pack_take(t, &rudi[trk_index(t)], slot); }

static void rudi_restart(void)                     /* PLAY: every rudiment from its hit 0, scene 1 */
{
    uint32_t i;
    if (!rudi_ready)
        rudi_init();
    for (i = 0; i < NTRK; i++) {
        rudi[i].hit = 0;
        rudi[i].sstep = 0;
        rudi[i].skip = 0;
        rudi[i].cur[0] = rudi[i].cur[1] = 0;
        rudi[i].pass = 0;
        if (trk[i].p[P_RUDCH] && trk[i].npack)     /* PACK: a lap from its first slot */
            rudi_pack_take(&trk[i], &rudi[i], 0);
        rudi[i].lastrud = (uint8_t)((uint32_t)trk[i].p[P_RUDI] % NUM_RUDIMENTS);
    }
}

#ifdef RUDI_STATS                       /* tests/rudi_test.c: what fired */
static uint32_t rudi_stat_steps[NTRK], rudi_stat_fired[NTRK][2];
static uint8_t rudi_stat_hand[64];
static uint32_t rudi_stat_n;
static uint8_t rudi_stat_note[64];      /* track 4's notes, bit 7 the hand (RUDI 3) */
static uint32_t rudi_stat_nn;
#define RUDI_STAT_STEP(k) (rudi_stat_steps[k]++)
#define RUDI_STAT_NOTE(k, hand, note) do { if ((k) == 3u && rudi_stat_nn < 64u) \
    rudi_stat_note[rudi_stat_nn++] = (uint8_t)(((note) & 127u) | ((hand) ? 128u : 0u)); } while (0)
#define RUDI_STAT_FIRE(k, hand) do { rudi_stat_fired[k][(hand) ? 1 : 0]++; \
    if ((k) == 0u && rudi_stat_n < 64u) rudi_stat_hand[rudi_stat_n++] = (uint8_t)((hand) ? 1 : 0); } while (0)
#else
#define RUDI_STAT_STEP(k) ((void)0)
#define RUDI_STAT_FIRE(k, hand) ((void)0)
#define RUDI_STAT_NOTE(k, hand, note) ((void)0)
#endif

/* a step of track t starts: the rudiment's next hit (seq_tick, in rudi mode) */
static __attribute__((noinline)) void rudi_step(track_t *t)
{
    rudi_lane_t *r = &rudi[trk_index(t)];
    uint32_t rud = (uint32_t)t->p[P_RUDI] % NUM_RUDIMENTS;
    const rudiment_t *rd = &RUDIMENTS[rud];
    const rudi_hit_t *h;
    uint32_t ss = r->sstep, note, every = (uint32_t)t->p[P_RUDION];
    /* RUDI 2X..4X: the lane steps every 2..4 track steps; between them the last hit keeps sounding (its gate is
     * one lane step) */
    if (r->skip) {
        r->skip--;
        return;
    }
    r->skip = (uint8_t)(every > 1u ? every - 1u : 0u);
    if (r->lastrud != rud) {                       /* the knob turned: the new sticking from its top */
        r->lastrud = (uint8_t)rud;
        r->hit = 0;
    }
    h = &rd->hits[r->hit % rd->nHits];
    RUDI_STAT_STEP(trk_index(t));
    r->hit = (uint8_t)((r->hit + 1u) % rd->nHits);
    if (r->hit == 0u)                              /* the pass ends with this hit: the PACK's next (CHAIN on) */
        rudi_pack_step(t, r);
    r->sstep = (uint8_t)((r->sstep + 1u) & 127u);  /* the smart clock runs through rests */
    rudi_hush(t);                                  /* the last hit's gate: one step */
    if (h->hand < 0)
        return;                                    /* a rest */
    if (!rudi_fires(t, r, h, ss))
        return;                                    /* rolled out */
    note = rudi_note(t, r, h->hand ? 1u : 0u);     /* NTE, or RUDI 3's random note of the scale */
    seq_note_on(t, note, rudi_vel(t, h));          /* the voice and MIDI OUT (seq.c) */
    RUDI_STAT_NOTE(trk_index(t), h->hand, note);
    r->sounding = (uint8_t)(note + 1u);
    RUDI_STAT_FIRE(trk_index(t), h->hand);
}
