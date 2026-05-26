/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

/*******************************************************************************
  Random placement of fixed-size squares inside a pixel region with a minimum
  inset from the outer rectangle. Tries to avoid overlap; if trials run out,
  places the square anyway (last random candidate, possibly overlapping).
 ******************************************************************************/

#ifndef PIXEL_SQUARE_PLACEMENT_H
#define PIXEL_SQUARE_PLACEMENT_H

#include <stdbool.h>

typedef struct {
    int x;
    int y;
    int w;
    int h;
} PixelsRect;

typedef struct {
    int h_start;       // left edge of region (inclusive pixel column)
    int h_end;         // right edge (exclusive): valid columns are h_start .. h_end-1
    int v_start;       // top edge (inclusive pixel row)
    int v_end;         // bottom edge (exclusive)
    int margin;        // squares must stay at least this many pixels inside h_/v_ bounds
    int square_side;   // width and height of each square (e.g. 144)
} PixelPlacementCfg;

// Half-open AABB overlap with strictly positive intersection area.
bool pixels_rect_overlap(const PixelsRect *a, const PixelsRect *b);

// True iff rect lies fully inside [h_start,h_end) x [v_start,v_end) with margin.
bool pixels_rect_inside_region(const PixelPlacementCfg *cfg, const PixelsRect *r);

// True iff r overlaps any rects[0..count-1].
bool pixels_rect_overlaps_any(const PixelsRect *r, const PixelsRect *rects, int count);

/*
  Randomly place n squares (side cfg->square_side) inside the margined region.
  Uses rand(); seed with srand() before calling if you want reproducibility.

  For each square, tries up to max_trials_per_square positions that do not overlap
  earlier entries. If none are found in time, accepts the last random try anyway
  (overlaps allowed). If max_trials_per_square is 0, skips the search and places
  one random position per square.

  @param cfg       region and margin (must satisfy usable width/height >= square_side)
  @param n         number of squares to place (may be 0)
  @param out       output array, length at least n
  @param max_trials_per_square  overlap-avoidance attempts per square (e.g. 50000)
  @return          n if region is valid, otherwise 0
 */
int pixels_place_squares_random(
    const PixelPlacementCfg *cfg,
    int n,
    PixelsRect *out,
    unsigned max_trials_per_square);

#endif
