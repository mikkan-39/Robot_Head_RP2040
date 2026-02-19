# Camera-iris eye renderer.
#
# Layer order (outside-in):
#   1  black background
#   2  secondary ring   — outer lens barrel, fixed radius
#   3  aperture blades  — primary fill, secondary edges, span full annulus
#   4  secondary ring   — inner aperture ring, radius driven by pupil_r
#   5  primary fill     — pupil
#   6  secondary ring   — small inner pupil ring
#
# Parallax depth model
# ──────────────────────────────────────────────────────────────────────────────
# Each layer has a depth factor P ∈ [0, 1]:
#   P = 0.0  → foreground (pupil)  — tracks gaze offset fully
#   P = 1.0  → background (outer ring) — barely moves
#
# Actual layer center:
#   cx_layer = display_cx + offset_x * (1 − P × PARALLAX_SCALE)
#   cy_layer = 120        + offset_y * (1 − P × PARALLAX_SCALE)
#
# where offset_x/y = commanded (cx,cy) − display center (120,120).
#
# PARALLAX_SCALE = 0.08 means at ±120 px max gaze, the outermost ring
# lags the pupil by 9.6 px.  Keep (P_a − P_b) × 120 × 0.08 < ring_width
# to avoid layers bleeding outside each other at extreme positions.
#
# Anti-aliasing
# ──────────────────────────────────────────────────────────────────────────────
# Each filled circle uses a ±0.6 px sub-pixel transition zone mapped to
# intermediate palette indices (PI_*) — pixels stay 4-bit indices.
# Blade edges fade across the outer 50 % of the blade width using the same indices.
#
# C port notes
# ──────────────────────────────────────────────────────────────────────────────
# - Two full canvas allocations (2 × 240×240 uint8 = 115 KB).
# - AA circles → integer dist² comparisons against (r±k)², no sqrt.
# - Blades → pre-baked polar texture; rotated via RP2040 interpolator.
# - Parallax → per-layer integer arithmetic, no extra memory.

import math
import numpy as np

from bitmap import (DisplayBitmap, WIDTH, HEIGHT,
                    PI_BG, PI_PRIMARY, PI_SECONDARY,
                    PI_BG_SEC_1,  PI_BG_SEC_2,  PI_BG_SEC_3,
                    PI_SEC_PRI_1, PI_SEC_PRI_2, PI_SEC_PRI_3,
                    PI_PRI_SEC_1, PI_PRI_SEC_2, PI_PRI_SEC_3)

# ── compile-time geometry constants ───────────────────────────────────────────
OUTER_RADIUS    = 70
OUTER_RING_W    =  3    # ring 2: outer lens barrel
APERTURE_RING_W =  2    # ring 4: inner aperture  (zoom-driven)
PUPIL_RING_W    =  2    # ring 6: pupil highlight

N_BLADES        =  8
BLADE_TILT      = math.pi * 0.55
BLADE_EDGE_PX   =  5
MAX_BLADE_ROT   = (2 * math.pi / N_BLADES) * 0.70

PUPIL_MIN_R     = 10
PUPIL_INNER_FR  = 0.60
PUPIL_RING_SCALE= 1.585

# Physical display offsets (binocular separation on the robot face)
R_EYE_OFFSET    = 15
L_EYE_OFFSET    = 15

DEFAULT_PUPIL_R = (OUTER_RADIUS - APERTURE_RING_W * 3 + PUPIL_MIN_R) // 2

DISPLAY_CX = WIDTH  // 2   # 120
DISPLAY_CY = HEIGHT // 2   # 120

# ── parallax depth factors ────────────────────────────────────────────────────
# P = 0 → foreground (full gaze tracking), P = 1 → background (barely moves)
# Rule: (P_a − P_b) × max_offset × PARALLAX_SCALE < min adjacent ring width
# At max_offset=120 and SCALE=0.08: max safe ΔP ≈ 0.25 (gives 2.4 px separation)
PARALLAX_SCALE  = -0.2
P_OUTER_RING    = 1.00
P_IRIS          = 0.75
P_BLADES        = 0.55
P_INNER_RING    = 0.25
P_PUPIL         = 0.00

# ── AA gradient table ─────────────────────────────────────────────────────────
_AA = {
    (PI_BG,        PI_SECONDARY): (PI_BG_SEC_1,  PI_BG_SEC_2,  PI_BG_SEC_3),
    (PI_SECONDARY, PI_PRIMARY):   (PI_SEC_PRI_1, PI_SEC_PRI_2, PI_SEC_PRI_3),
    (PI_PRIMARY,   PI_SECONDARY): (PI_PRI_SEC_1, PI_PRI_SEC_2, PI_PRI_SEC_3),
}


# ── public API ────────────────────────────────────────────────────────────────
def draw_camera_eyes(right: DisplayBitmap, left: DisplayBitmap,
                     cx: int, cy: int, pupil_r: int):
    """
    Render both eyes with per-layer parallax.

    cx, cy  — gaze target in display coordinates (120, 120 = center).
              Both eyes track the same direction; left eye uses swirl = −1.
    """
    offset_x = cx - DISPLAY_CX
    offset_y = cy - DISPLAY_CY

    right_canvas = np.zeros((HEIGHT, WIDTH), dtype=np.uint8)
    _draw_eye(right_canvas, DISPLAY_CX + R_EYE_OFFSET,
              offset_x, offset_y, pupil_r, swirl=1)
    right.set_from_array(right_canvas)

    left_canvas = np.zeros((HEIGHT, WIDTH), dtype=np.uint8)
    _draw_eye(left_canvas,  DISPLAY_CX - L_EYE_OFFSET,
              offset_x, offset_y, pupil_r, swirl=-1)
    left.set_from_array(left_canvas)


# ── internals ─────────────────────────────────────────────────────────────────
def _lpos(display_cx: int, offset_x: int, offset_y: int,
          p: float) -> tuple[int, int]:
    """
    Return (cx, cy) for a layer at depth factor p.
    Layers with high P lag behind; layers with P=0 track the gaze fully.
    """
    f = 1.0 - p * PARALLAX_SCALE
    return (round(display_cx + offset_x * f),
            round(DISPLAY_CY  + offset_y * f))


def _draw_eye(canvas: np.ndarray,
              display_cx: int, offset_x: int, offset_y: int,
              pupil_r: int, swirl: int):
    max_pupil_r = OUTER_RADIUS - OUTER_RING_W - APERTURE_RING_W - 2
    pupil_r     = int(np.clip(pupil_r, PUPIL_MIN_R, max_pupil_r))

    t_close    = 1.0 - (pupil_r - PUPIL_MIN_R) / max(max_pupil_r - PUPIL_MIN_R, 1)
    blade_rot  = t_close * MAX_BLADE_ROT      # swirl applied inside _draw_blades

    iris_r     = OUTER_RADIUS - OUTER_RING_W
    pupil_fill = max(3, pupil_r - APERTURE_RING_W)
    pupil_hi_r = max(4, int(PUPIL_INNER_FR * max_pupil_r
                            * (pupil_r / max_pupil_r) ** PUPIL_RING_SCALE))
    pupil_hi_f = max(2, pupil_hi_r - PUPIL_RING_W)

    # Per-layer centers (parallax applied)
    cx_o, cy_o = _lpos(display_cx, offset_x, offset_y, P_OUTER_RING)
    cx_i, cy_i = _lpos(display_cx, offset_x, offset_y, P_IRIS)
    cx_b, cy_b = _lpos(display_cx, offset_x, offset_y, P_BLADES)
    cx_r, cy_r = _lpos(display_cx, offset_x, offset_y, P_INNER_RING)
    cx_p, cy_p = _lpos(display_cx, offset_x, offset_y, P_PUPIL)

    # 2  outer ring (hard outer edge — on black BG, AA there shifts ring colour)
    _fill_aa(canvas, cx_o, cy_o, OUTER_RADIUS, PI_SECONDARY, PI_BG, aa_outer=False)
    # 3a iris fill
    _fill_aa(canvas, cx_i, cy_i, iris_r, PI_PRIMARY, PI_SECONDARY)

    # 3b aperture blades
    blade_inner = pupil_r + APERTURE_RING_W
    if blade_inner < iris_r - 2:
        _draw_blades(canvas, cx_b, cy_b, iris_r, blade_inner, blade_rot, swirl)

    # 4  inner aperture ring
    _fill_aa(canvas, cx_r, cy_r, pupil_r,    PI_SECONDARY, PI_PRIMARY)
    _fill_aa(canvas, cx_r, cy_r, pupil_fill, PI_PRIMARY,   PI_SECONDARY)

    # 6  pupil highlight ring
    if pupil_hi_r > pupil_hi_f + 1:
        _fill_aa(canvas, cx_p, cy_p, pupil_hi_r, PI_SECONDARY, PI_PRIMARY)
        _fill_aa(canvas, cx_p, cy_p, pupil_hi_f, PI_PRIMARY,   PI_SECONDARY)


# ── drawing primitives ────────────────────────────────────────────────────────
def _fill_aa(canvas: np.ndarray, cx: int, cy: int, r: int,
             fill_idx: int, bg_idx: int, aa_outer: bool = True):
    """
    Fill circle of radius r, with a tight ~1.2 px AA edge.

    d = signed pixel distance from edge (− inside, + outside):
      d < −0.6            → fill_idx
      −0.6 ≤ d < −0.2    → aa75
      −0.2 ≤ d < +0.2    → aa50
      +0.2 ≤ d < +0.6    → aa25   (skipped when aa_outer=False)
      d ≥  +0.6           → untouched
    """
    grad = _AA.get((bg_idx, fill_idx))
    h, w = canvas.shape
    pad  = 2
    x0 = max(cx - r - pad, 0);  y0 = max(cy - r - pad, 0)
    x1 = min(cx + r + pad, w);  y1 = min(cy + r + pad, h)
    if x1 <= x0 or y1 <= y0:
        return

    ys, xs = np.mgrid[y0:y1, x0:x1]
    d = (np.sqrt((xs.astype(np.float32) - cx) ** 2 +
                 (ys.astype(np.float32) - cy) ** 2) - r)

    sub = canvas[y0:y1, x0:x1]
    sub[d < -0.6] = fill_idx
    if grad is None:
        return
    aa25, aa50, aa75 = grad
    sub[(d >= -0.6) & (d < -0.2)] = aa75
    sub[(d >= -0.2) & (d <  0.2)] = aa50
    if aa_outer:
        sub[(d >= 0.2) & (d < 0.6)] = aa25


def _draw_blades(canvas: np.ndarray,
                 cx: int, cy: int,
                 outer_r: int, inner_r: int,
                 blade_rot: float, swirl: int = 1):
    """
    Paint N aperture blade edges in [inner_r, outer_r] with AA.

    swirl = ±1 mirrors the spiral direction for left/right eyes.
    blade_rot drives the aperture opening; swirl * BLADE_TILT drives the spiral curve.

    Inner 50 % of each blade stripe → solid SECONDARY.
    Outer 50 % fades PRIMARY → SECONDARY over 3 steps.
    """
    h, w = canvas.shape
    x0 = max(cx - outer_r, 0); y0 = max(cy - outer_r, 0)
    x1 = min(cx + outer_r, w); y1 = min(cy + outer_r, h)
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
    blade_pos = (phi - blade_rot - swirl * BLADE_TILT * t) % sector
    edge      = BLADE_EDGE_PX / np.maximum(r, 1.0)

    frac     = blade_pos / np.maximum(edge, 1e-6)
    in_blade = in_ring & (frac < 1.0)

    sub = canvas[y0:y1, x0:x1]
    sub[in_blade & (frac <  0.5)]                   = PI_SECONDARY
    sub[in_blade & (frac >= 0.5) & (frac < 0.67)]  = PI_PRI_SEC_3
    sub[in_blade & (frac >= 0.67) & (frac < 0.83)] = PI_PRI_SEC_2
    sub[in_blade & (frac >= 0.83)]                  = PI_PRI_SEC_1
