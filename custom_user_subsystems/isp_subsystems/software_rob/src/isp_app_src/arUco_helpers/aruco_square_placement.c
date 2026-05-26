/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "aruco_square_placement.h"
#include <stdlib.h>

bool pixels_rect_overlap(const PixelsRect *a, const PixelsRect *b) {
    /* Separating axis => no overlap */
    if (a->x + a->w <= b->x) {
        return false;
    }
    if (b->x + b->w <= a->x) {
        return false;
    }
    if (a->y + a->h <= b->y) {
        return false;
    }
    if (b->y + b->h <= a->y) {
        return false;
    }
    return true;

}

bool pixels_rect_inside_region(const PixelPlacementCfg *cfg, const PixelsRect *r) {

    const int inner_left = cfg->h_start + cfg->margin;
    const int inner_top = cfg->v_start + cfg->margin;
    const int inner_right = cfg->h_end - cfg->margin;
    const int inner_bottom = cfg->v_end - cfg->margin;

    return r->x >= inner_left
        && r->y >= inner_top
        && r->x + r->w <= inner_right
        && r->y + r->h <= inner_bottom;

}

bool pixels_rect_overlaps_any(const PixelsRect *r, const PixelsRect *rects, int count) {

    for (int i = 0; i < count; i++) {
        if (pixels_rect_overlap(r, &rects[i])) {
            return true;
        }
    }
    return false;

}

static int rand_range_inclusive(int lo, int hi) {

    /* hi >= lo; inclusive range */
    unsigned span = (unsigned)(hi - lo + 1);
    return lo + (int)((unsigned)rand() % span);

}

int pixels_place_squares_random(
    const PixelPlacementCfg *cfg,
    int n,
    PixelsRect *out,
    unsigned max_trials_per_square)
{

    const int s = cfg->square_side;

    if (n <= 0 || s <= 0) {
        return 0;
    }

    const int inner_left = cfg->h_start + cfg->margin;
    const int inner_top = cfg->v_start + cfg->margin;
    const int inner_right = cfg->h_end - cfg->margin;
    const int inner_bottom = cfg->v_end - cfg->margin;

    const int x_max = inner_right - s;
    const int y_max = inner_bottom - s;

    if (inner_left > x_max || inner_top > y_max) {
        return 0;
    }

    int placed = 0;
    for (int k = 0; k < n; k++) {
        PixelsRect cand = { 0, 0, s, s };
        bool found_clear = false;
        unsigned trials = 0;

        for (; trials < max_trials_per_square; trials++) {
            cand.x = rand_range_inclusive(inner_left, x_max);
            cand.y = rand_range_inclusive(inner_top, y_max);
            if (!pixels_rect_overlaps_any(&cand, out, placed)) {
                found_clear = true;
                break;
            }
        }

        // No non-overlap found in time: still commit last random cand (may overlap).
        //   If max_trials_per_square == 0, inner loop never ran — pick one random spot.
        if (!found_clear && trials == 0) {
            cand.x = rand_range_inclusive(inner_left, x_max);
            cand.y = rand_range_inclusive(inner_top, y_max);
        }

        out[placed++] = cand;
    }

    return placed;

}
