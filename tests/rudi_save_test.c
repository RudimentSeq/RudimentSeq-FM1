/* SPDX-License-Identifier: GPL-3.0-only
 * RudimentSeq (rudimentseq.com), 2026-11-18. Host test of the rudiment lane through the stores:
 *   - SAVE > PROJECT, named: the eight lane values (P_RUDION .. P_RUDNL) and the name come back on load, twice;
 *   - SAVE > USER, named, with RUDI on: the sound carries its lane (upreset.c up_load with_rudi); SAVE held
 *     (undo_swap) puts the track's own lane back and, held again, the sound's again;
 *   - a user sound saved with RUDI off leaves the track's lane alone (Felucca's rule, param_kept);
 *   - saving while playing is refused (STOP TO SAVE), the slot stays empty.
 * Built and run by tests/run_tests.sh. */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"

static int fails;
static void chk(const char *what, int ok) { printf("%s  %s\n", ok ? "ok  " : "FAIL", what); fails += !ok; }
static void lane_set(track_t *t, int on, int rud, int nr, int nl, int pr, int pl, int sma, int fea)
{
    t->p[P_RUDION] = (int16_t)on; t->p[P_RUDI] = (int16_t)rud; t->p[P_RUDNR] = (int16_t)nr; t->p[P_RUDNL] = (int16_t)nl;
    t->p[P_RUDPR] = (int16_t)pr; t->p[P_RUDPL] = (int16_t)pl; t->p[P_RUDSMA] = (int16_t)sma; t->p[P_RUDFEA] = (int16_t)fea;
}
static int lane_is(const track_t *t, int on, int rud, int nr, int nl, int pr, int pl, int sma, int fea)
{
    return t->p[P_RUDION] == on && t->p[P_RUDI] == rud && t->p[P_RUDNR] == nr && t->p[P_RUDNL] == nl &&
           t->p[P_RUDPR] == pr && t->p[P_RUDPL] == pl && t->p[P_RUDSMA] == sma && t->p[P_RUDFEA] == fea;
}

int main(void)
{
    track_t *t;
    int16_t v[P_COUNT];
    char nm[16];
    int rc;
    ui_power_on();
    frames(3000);                                       /* past the splash; stopped */
    song.sel = 0;
    t = &trk[0];
    chk("transport stopped at the start", !transport_busy());

    /* ---- the project */
    lane_set(t, 1, 42, 60, 48, 80, 55, 1, 30);
    chk("project_save_as(1, PARA) == 0", project_save_as(1, "PARA") == 0);
    lane_set(t, 0, 0, 1, 2, 100, 100, 0, 0);
    project_load(1);
    t = &trk[0];
    chk("project_load(1): the lane back (RUDI on, 42, notes 60/48, PRB 80/55, SMART, FTHR 30)", lane_is(t, 1, 42, 60, 48, 80, 55, 1, 30));
    project_cur_name(nm);
    chk("the project's name PARA after the load", !strcmp(nm, "PARA"));
    chk("project_name(1) reads PARA from the slot", project_name(1, nm) != 0 && !strcmp(nm, "PARA"));
    memset(&trk[0].p[P_RUDION], 0, (P_E0 - P_RUDION) * sizeof(int16_t));
    project_load(1);
    chk("a second load restores the lane again", lane_is(&trk[0], 1, 42, 60, 48, 80, 55, 1, 30));

    /* ---- a user sound saved with RUDI on */
    t = &trk[0];
    lane_set(t, 1, 42, 60, 48, 80, 55, 1, 30);
    rc = up_store(3, "RUDBASS");
    chk("up_store(3, RUDBASS) stored (0 flash, 3 RAM)", rc == 0 || rc == 3);
    up_values(up_rec(3), v);
    chk("the record holds the lane", v[P_RUDION] == 1 && v[P_RUDI] == 42 && v[P_RUDNR] == 60 && v[P_RUDFEA] == 30);
    lane_set(t, 0, 7, 99, 98, 10, 20, 0, 5);            /* the track's own lane, as a fresh track or another project */
    chk("up_load(3) == 0", up_load(3) == 0);
    chk("the sound brings its lane: RUDI on, 42, 60/48, 80/55, SMART, FTHR 30", lane_is(t, 1, 42, 60, 48, 80, 55, 1, 30));
    undo_swap();                                        /* SAVE held */
    chk("SAVE held: the track's own lane back (off, 7, 99/98, 10/20, 0, 5)", lane_is(t, 0, 7, 99, 98, 10, 20, 0, 5));
    undo_swap();
    chk("SAVE held again: the sound's lane again", lane_is(t, 1, 42, 60, 48, 80, 55, 1, 30));

    /* ---- a user sound saved with RUDI off leaves the lane alone */
    lane_set(t, 0, 9, 50, 51, 100, 100, 0, 0);
    rc = up_store(4, "PLAIN");
    chk("up_store(4, PLAIN) stored", rc == 0 || rc == 3);
    lane_set(t, 1, 42, 60, 48, 80, 55, 1, 30);
    chk("up_load(4) == 0", up_load(4) == 0);
    chk("a sound saved with RUDI off leaves the track's lane as it was", lane_is(t, 1, 42, 60, 48, 80, 55, 1, 30));
    undo_swap();
    chk("SAVE held after it: the lane still as it was (nothing of it to undo)", lane_is(t, 1, 42, 60, 48, 80, 55, 1, 30));

    /* ---- saving while playing */
    btn_down(B_PLAY); frame(); btn_up(B_PLAY); frames(200);
    chk("transport running after PLAY", transport_busy());
    chk("project_save_as while playing returns 1 (STOP TO SAVE)", project_save_as(2, "X") == 1);
    chk("slot C stays empty", !project_used(2));

    printf("%s: %d failure(s)\n", fails ? "RESULT FAIL" : "RESULT PASS", fails);
    return fails != 0;
}
