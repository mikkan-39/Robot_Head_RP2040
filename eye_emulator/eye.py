# Camera-iris eye renderer.
#
# Layer order (outside-in):
#   1  black background
#   2  secondary ring        — outer lens barrel, fixed radius
#   3  aperture blades       — primary fill, secondary edges, span full annulus
#   4  secondary ring        — inner aperture ring, radius driven by pupil_r
#   5  primary fill          — pupil
#   6  secondary ring        — small inner pupil ring
#
# pupil_r (from DRAW_EYES) controls elements 4-6 size AND rotates the blades.
# Large pupil_r = open aperture (blades barely rotated).
# Small pupil_r = closed aperture (blades rotated inward).
#
# Anti-aliasing:
#   Each filled circle is drawn with a 2-pixel smooth transition zone.
#   The transition is encoded as intermediate palette indices (PI_*) so the
#   pixel value stays a 4-bit index — no per-pixel blending at display time.
#   Three AA gradients cover all transitions in the eye:
#     BG → SECONDARY     (outer ring outer edge)
#     SECONDARY → PRIMARY (outer ring inner edge, inner ring inner edge)
#     PRIMARY → SECONDARY (inner ring outer edge, blade edges)
#
# C port notes:
#   - This file uses numpy + trig freely; that is Python-only.
#   - AA circle → integer dist² comparisons against (r±k)² thresholds, no sqrt.
#   - Blade pattern pre-baked into a texture; rotated each frame via interpolator.
#   - The one cos/sin call per frame sets interp base[0/1]; inner loop = POP[2].

import math
import numpy as np

from bitmap import (DisplayBitmap, WIDTH, HEIGHT,
                    PI_BG, PI_PRIMARY, PI_SECONDARY, PI_RESERVED,
                    PI_BG_SEC_1,  PI_BG_SEC_2,  PI_BG_SEC_3,
                    PI_SEC_PRI_1, PI_SEC_PRI_2, PI_SEC_PRI_3,
                    PI_PRI_SEC_1, PI_PRI_SEC_2, PI_PRI_SEC_3)

# ── compile-time eye geometry constants ──────────────────────────────────────
OUTER_RADIUS     = 70
OUTER_RING_W     =  3    # pixel width of ring 2 (outer lens barrel)
APERTURE_RING_W  =  2    # pixel width of ring 4 (inner aperture, zoom-driven)
PUPIL_RING_W     =  2    # pixel width of ring 6 (small inner pupil highlight)

N_BLADES         =  8
BLADE_TILT       = math.pi * 0.55
BLADE_EDGE_PX    = 5
MAX_BLADE_ROT    = (2 * math.pi / N_BLADES) * 0.70

PUPIL_MIN_R      = 10
PUPIL_INNER_FR   = 0.60
PUPIL_RING_SCALE = 1.585

R_EYE_OFFSET     = 15
L_EYE_OFFSET     = 15

DEFAULT_PUPIL_R  = (OUTER_RADIUS - APERTURE_RING_W * 3 + PUPIL_MIN_R) // 2

# ── AA gradient table ─────────────────────────────────────────────────────────
# Maps (bg_index, fill_index) → (aa25, aa50, aa75) palette indices.
# aa25 = 25 % toward fill (placed just outside the edge),
# aa75 = 75 % toward fill (placed just inside the edge).
_AA = {
    (PI_BG,        PI_SECONDARY): (PI_BG_SEC_1,  PI_BG_SEC_2,  PI_BG_SEC_3),
    (PI_SECONDARY, PI_PRIMARY):   (PI_SEC_PRI_1, PI_SEC_PRI_2, PI_SEC_PRI_3),
    (PI_PRIMARY,   PI_SECONDARY): (PI_PRI_SEC_1, PI_PRI_SEC_2, PI_PRI_SEC_3),
}


# ── public API ────────────────────────────────────────────────────────────────
def draw_camera_eyes(right: DisplayBitmap, left: DisplayBitmap,
                     cx: int, cy: int, pupil_r: int):
    _draw_eye(right, cx + R_EYE_OFFSET, cy, pupil_r, swirl= 1)
    _draw_eye(left,  cx - L_EYE_OFFSET, cy, pupil_r, swirl=-1)


# ── internals ─────────────────────────────────────────────────────────────────
def _draw_eye(bm: DisplayBitmap, cx: int, cy: int,
              pupil_r: int, swirl: int = 1):
    max_pupil_r = OUTER_RADIUS - OUTER_RING_W - APERTURE_RING_W - 2
    pupil_r     = int(np.clip(pupil_r, PUPIL_MIN_R, max_pupil_r))

    t_close    = 1.0 - (pupil_r - PUPIL_MIN_R) / max(max_pupil_r - PUPIL_MIN_R, 1)
    blade_rot  = swirl * t_close * MAX_BLADE_ROT

    iris_r      = OUTER_RADIUS  - OUTER_RING_W
    pupil_fill  = max(3, pupil_r - APERTURE_RING_W)
    pupil_hi_r  = max(4, int(PUPIL_INNER_FR * max_pupil_r
                             * (pupil_r / max_pupil_r) ** PUPIL_RING_SCALE))
    pupil_hi_f  = max(2, pupil_hi_r - PUPIL_RING_W)

    canvas = np.zeros((HEIGHT, WIDTH), dtype=np.uint8)  # all PI_BG

    # ── 2  outer ring ─────────────────────────────────────────────────────────
    # Hard outer edge: sits on black BG, dark-on-dark — AA there shifts the ring
    # colour by blending in BG pixels and makes it look different from inner rings.
    _fill_aa(canvas, cx, cy, OUTER_RADIUS, PI_SECONDARY, PI_BG, aa_outer=False)
    _fill_aa(canvas, cx, cy, iris_r,       PI_PRIMARY,   PI_SECONDARY)

    # ── 3  aperture blades (hard edge — already thin) ─────────────────────────
    blade_inner = pupil_r + APERTURE_RING_W
    if blade_inner < iris_r - 2:
        _draw_blades(canvas, cx, cy, iris_r, blade_inner, blade_rot, swirl)

    # ── 4  inner aperture ring ────────────────────────────────────────────────
    _fill_aa(canvas, cx, cy, pupil_r,    PI_SECONDARY, PI_PRIMARY)
    _fill_aa(canvas, cx, cy, pupil_fill, PI_PRIMARY,   PI_SECONDARY)

    # ── 6  pupil inner highlight ring ─────────────────────────────────────────
    if pupil_hi_r > pupil_hi_f + 1:
        _fill_aa(canvas, cx, cy, pupil_hi_r, PI_SECONDARY, PI_PRIMARY)
        _fill_aa(canvas, cx, cy, pupil_hi_f, PI_PRIMARY,   PI_SECONDARY)

    bm.set_from_array(canvas)


def _fill_aa(canvas: np.ndarray, cx: int, cy: int, r: int,
             fill_idx: int, bg_idx: int, aa_outer: bool = True):
    """
    Fill circle of radius r with fill_idx, with a tight ~1.2px AA inner edge.

    Transition zones (d = signed distance from circle edge, + = outside):
      d < -0.6            → fill_idx  (solid interior)
      -0.6 ≤ d < -0.2    → aa75
      -0.2 ≤ d < +0.2    → aa50
      +0.2 ≤ d < +0.6    → aa25      (only when aa_outer=True)
      d ≥ +0.6            → leave as-is

    aa_outer=False: skip the outer AA band (d > 0).  Use this when the circle
    sits on a dark/black background — the hard outer edge is imperceptible and
    avoids a visual colour shift caused by bg-blended AA pixels.

    C port: replace sqrt with integer dist² checks against (r±k)².
    """
    grad = _AA.get((bg_idx, fill_idx))

    pad = 2
    x0 = max(cx - r - pad, 0);     y0 = max(cy - r - pad, 0)
    x1 = min(cx + r + pad, WIDTH); y1 = min(cy + r + pad, HEIGHT)
    if x1 <= x0 or y1 <= y0:
        return

    ys, xs = np.mgrid[y0:y1, x0:x1]
    dx   = xs.astype(np.float32) - cx
    dy   = ys.astype(np.float32) - cy
    dist = np.sqrt(dx * dx + dy * dy)
    d    = dist - r   # negative = inside, positive = outside

    sub = canvas[y0:y1, x0:x1]
    sub[d < -0.6] = fill_idx

    if grad is None:
        return

    aa25, aa50, aa75 = grad
    sub[(d >= -0.6) & (d < -0.2)] = aa75
    sub[(d >= -0.2) & (d <  0.2)] = aa50
    if aa_outer:
        sub[(d >= 0.2) & (d < 0.6)] = aa25
    # d >= 0.6 (or d >= 0.2 when aa_outer=False): untouched


def _draw_blades(canvas: np.ndarray,
                 cx: int, cy: int,
                 outer_r: int, inner_r: int,
                 blade_rot: float, swirl: int = 1):
    """
    Paint N evenly-spaced curved blade edges in the annulus with AA.

    Each blade stripe is BLADE_EDGE_PX wide at each radius.
    The outer half fades from SECONDARY → PRIMARY using the gradient,
    giving a smooth edge without visible jaggies.
    """
    x0 = max(cx - outer_r, 0); y0 = max(cy - outer_r, 0)
    x1 = min(cx + outer_r, WIDTH); y1 = min(cy + outer_r, HEIGHT)
    if x1 <= x0 or y1 <= y0:
        return

    ys, xs = np.mgrid[y0:y1, x0:x1]
    dx = xs.astype(np.float32) - cx
    dy = ys.astype(np.float32) - cy
    r2 = dx * dx + dy * dy
    r  = np.sqrt(r2)

    in_ring   = (r2 >= inner_r * inner_r) & (r2 <= outer_r * outer_r)
    phi       = np.arctan2(dy, dx)
    t         = np.clip((r - inner_r) / float(max(outer_r - inner_r, 1)), 0.0, 1.0)
    sector    = (2.0 * math.pi) / N_BLADES
    blade_pos = (phi - swirl * (blade_rot + BLADE_TILT * t)) % sector
    edge      = BLADE_EDGE_PX / np.maximum(r, 1.0)

    # frac ∈ [0, 1) within the blade stripe; frac ≥ 1 = outside (PRIMARY body)
    frac     = blade_pos / np.maximum(edge, 1e-6)
    in_blade = in_ring & (frac < 1.0)

    sub = canvas[y0:y1, x0:x1]
    sub[in_blade & (frac < 0.5)]                         = PI_SECONDARY
    sub[in_blade & (frac >= 0.5) & (frac < 0.67)]        = PI_PRI_SEC_3
    sub[in_blade & (frac >= 0.67) & (frac < 0.83)]       = PI_PRI_SEC_2
    sub[in_blade & (frac >= 0.83)]                        = PI_PRI_SEC_1
