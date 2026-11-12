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

static uint32_t rudi_rand100(rudi_lane_t *r)
{
    r->rng ^= r->rng << 13; r->rng ^= r->rng >> 17; r->rng ^= r->rng << 5;
    return (r->rng >> 16) % 100u;
}

static void rudi_hush(track_t *t)                  /* the held note off (stop, mode off) */
{
    rudi_lane_t *r = &rudi[trk_index(t)];
    if (r->sounding) {
        seq_note_off(t, r->sounding - 1u);         /* the voice and MIDI OUT (seq.c) */
        r->sounding = 0;
    }
}

static void rudi_restart(void)                     /* PLAY: every rudiment from its hit 0, scene 1 */
{
    uint32_t i;
    if (!rudi_ready)
        rudi_init();
    for (i = 0; i < NTRK; i++) {
        rudi[i].hit = 0;
        rudi[i].sstep = 0;
        rudi[i].lastrud = (uint8_t)((uint32_t)trk[i].p[P_RUDI] % NUM_RUDIMENTS);
    }
}

#ifdef RUDI_STATS                       /* tests/rudi_test.c: what fired */
static uint32_t rudi_stat_steps[NTRK], rudi_stat_fired[NTRK][2];
static uint8_t rudi_stat_hand[64];
static uint32_t rudi_stat_n;
#define RUDI_STAT_STEP(k) (rudi_stat_steps[k]++)
#define RUDI_STAT_FIRE(k, hand) do { rudi_stat_fired[k][(hand) ? 1 : 0]++; \
    if ((k) == 0u && rudi_stat_n < 64u) rudi_stat_hand[rudi_stat_n++] = (uint8_t)((hand) ? 1 : 0); } while (0)
#else
#define RUDI_STAT_STEP(k) ((void)0)
#define RUDI_STAT_FIRE(k, hand) ((void)0)
#endif

/* a step of track t starts: the rudiment's next hit (seq_tick, in rudi mode) */
static __attribute__((noinline)) void rudi_step(track_t *t)
{
    rudi_lane_t *r = &rudi[trk_index(t)];
    uint32_t rud = (uint32_t)t->p[P_RUDI] % NUM_RUDIMENTS;
    const rudiment_t *rd = &RUDIMENTS[rud];
    const rudi_hit_t *h;
    uint32_t ss = r->sstep, p, vel, note, feather;
    if (r->lastrud != rud) {                       /* the knob turned: the new sticking from its top */
        r->lastrud = (uint8_t)rud;
        r->hit = 0;
    }
    h = &rd->hits[r->hit % rd->nHits];
    RUDI_STAT_STEP(trk_index(t));
    r->hit = (uint8_t)((r->hit + 1u) % rd->nHits);
    r->sstep = (uint8_t)((r->sstep + 1u) & 127u);  /* the smart clock runs through rests */
    rudi_hush(t);                                  /* the last hit's gate: one step */
    if (h->hand < 0)
        return;                                    /* a rest */
    p = (uint32_t)(h->hand ? t->p[P_RUDPL] : t->p[P_RUDPR]);
    if (t->p[P_RUDSMA])
        p = p * rudi_smart[h->hand ? 1 : 0][ss] / 100u;
    if (p < 100u && rudi_rand100(r) >= p)
        return;                                    /* rolled out */
    vel = 90;                                      /* the web app's velocity model (0..127) */
    feather = (uint32_t)t->p[P_RUDFEA];
    if (h->accent)
        vel = 108;                                 /* 90 + (115 - 90) x 90 / 127 */
    else if (feather)
        vel = 90u - (90u - 20u) * feather / 100u;
    if (h->grace)
        vel = 30;
    note = (uint32_t)(h->hand ? t->p[P_RUDNL] : t->p[P_RUDNR]) & 127u;
    seq_note_on(t, note, vel);                     /* the voice and MIDI OUT (seq.c) */
    r->sounding = (uint8_t)(note + 1u);
    RUDI_STAT_FIRE(trk_index(t), h->hand);
}
