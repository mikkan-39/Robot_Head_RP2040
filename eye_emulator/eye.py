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
# C port notes:
#   - This file uses numpy + trig freely; that is Python-only.
#   - In C, the blade pattern will be pre-baked into a texture and rotated
#     each frame using the RP2040 interpolator (see st7789_lcd.c pattern).
#     The one cos/sin call per frame sets interp base[0/1]; the inner loop
#     becomes a single POP[2] per pixel with zero per-pixel CPU involvement.
#   - All other layers (circles, rings) port directly using integer arithmetic.

import math
import numpy as np

from bitmap import DisplayBitmap, WIDTH, HEIGHT, BG, PRIMARY, SECONDARY, RESERVED

# ── compile-time eye geometry constants ─────────────────────────────────────
# Change these to tune the look; they are NOT sent over the command protocol.

OUTER_RADIUS     = 70    # outer lens ring outer edge — does NOT zoom
OUTER_RING_W     =  5    # pixel width of ring 2 (outer lens barrel)
APERTURE_RING_W  =  3    # pixel width of ring 4 (inner aperture, zoom-driven)
PUPIL_RING_W     =  2    # pixel width of ring 6 (small inner pupil highlight)

N_BLADES         =  8    # aperture blade count
BLADE_TILT       = math.pi * 0.1  # angular sweep of blade curve, inner→outer
BLADE_EDGE_PX    = 2               # blade edge width in screen pixels (approx)
MAX_BLADE_ROT    = (2 * math.pi / N_BLADES) * 0.70  # blade rotation at fully closed

PUPIL_MIN_R      = 6    # smallest allowed pupil_r
PUPIL_INNER_FR   = 0.30  # ring 6 radius at fully open = max_pupil_r * this
PUPIL_RING_SCALE = 0.85 # power-law exponent: ring6 = PUPIL_INNER_FR * max_r * (r/max_r)^SCALE
                         #   1.0   = linear, ring 6 tracks ring 4 1:1
                         #   1.585 = aperture 2× → pupil 3×  (= log₂3, matches user spec)
                         #   2.0   = aperture 2× → pupil 4×
                         #   <1.0  = ring 6 barely moves (stable inner dot)

R_EYE_OFFSET     = 15
L_EYE_OFFSET     = 15

# Convenience: a half-open aperture makes a good startup default
DEFAULT_PUPIL_R  = (OUTER_RADIUS - APERTURE_RING_W * 3 + PUPIL_MIN_R) // 2


# ── public API ───────────────────────────────────────────────────────────────

def draw_camera_eyes(right: DisplayBitmap, left: DisplayBitmap,
                     cx: int, cy: int, pupil_r: int):
    """Render both displays.  pupil_r drives aperture size and blade angle."""
    _draw_eye(right, cx + R_EYE_OFFSET, cy, pupil_r, swirl=-1)
    _draw_eye(left,  cx - L_EYE_OFFSET, cy, pupil_r, swirl= 1)


# ── internals ────────────────────────────────────────────────────────────────

def _draw_eye(bm: DisplayBitmap, cx: int, cy: int, pupil_r: int, swirl: int = 1):
    max_pupil_r = OUTER_RADIUS - OUTER_RING_W - APERTURE_RING_W - 2
    pupil_r     = int(np.clip(pupil_r, PUPIL_MIN_R, max_pupil_r))

    # aperture openness: 0 = wide open (large pupil), 1 = fully closed
    t_close   = 1.0 - (pupil_r - PUPIL_MIN_R) / max(max_pupil_r - PUPIL_MIN_R, 1)
    blade_rot = swirl * t_close * MAX_BLADE_ROT

    iris_r      = OUTER_RADIUS  - OUTER_RING_W     # inner edge of ring 2
    pupil_fill  = max(3, pupil_r - APERTURE_RING_W) # inner edge of ring 4
    # Power-law: ring 6 anchored to open size, shrinks faster than ring 4.
    # At pupil_r = max_pupil_r: ring6 = PUPIL_INNER_FR * max_pupil_r
    # At pupil_r = max/2:       ring6 shrinks by (0.5)^SCALE of that open value
    pupil_hi_r  = max(4, int(PUPIL_INNER_FR * max_pupil_r
                             * (pupil_r / max_pupil_r) ** PUPIL_RING_SCALE))
    pupil_hi_f  = max(2, pupil_hi_r - PUPIL_RING_W)

    canvas = np.zeros((HEIGHT, WIDTH), dtype=np.uint8)

    # ── 2  outer ring ────────────────────────────────────────────────────────
    _fill_circle(canvas, cx, cy, OUTER_RADIUS, SECONDARY)
    _fill_circle(canvas, cx, cy, iris_r,       PRIMARY)

    # ── 3  aperture blades ───────────────────────────────────────────────────
    blade_inner = pupil_r + APERTURE_RING_W
    if blade_inner < iris_r - 2:
        _draw_blades(canvas, cx, cy, iris_r, blade_inner, blade_rot, swirl)

    # ── 4  inner aperture ring ───────────────────────────────────────────────
    _fill_circle(canvas, cx, cy, pupil_r,    SECONDARY)
    _fill_circle(canvas, cx, cy, pupil_fill, PRIMARY)

    # ── 6  pupil inner highlight ring ────────────────────────────────────────
    if pupil_hi_r > pupil_hi_f + 1:
        _fill_circle(canvas, cx, cy, pupil_hi_r, SECONDARY)
        _fill_circle(canvas, cx, cy, pupil_hi_f, PRIMARY)

    bm.set_from_array(canvas)


def _fill_circle(canvas: np.ndarray, cx: int, cy: int, r: int, color: int):
    x0 = max(cx - r, 0);     y0 = max(cy - r, 0)
    x1 = min(cx + r, WIDTH); y1 = min(cy + r, HEIGHT)
    if x1 <= x0 or y1 <= y0:
        return
    ys, xs = np.mgrid[y0:y1, x0:x1]
    dx = xs.astype(np.float32) - cx
    dy = ys.astype(np.float32) - cy
    canvas[y0:y1, x0:x1][dx * dx + dy * dy <= r * r] = color


def _draw_blades(canvas: np.ndarray,
                 cx: int, cy: int,
                 outer_r: int, inner_r: int,
                 blade_rot: float, swirl: int = 1):
    """
    Paint N evenly-spaced curved blade edges (SECONDARY) in the annulus.

    Blades tile the full 360° (no gaps).  The only visible mark per blade is
    its single sector boundary: a curved line whose angular position shifts
    linearly with radius (BLADE_TILT), producing the swirl look.

    C translation:
        blade_rot is computed once per frame from pupil_r (LUT or shift).
        The per-pixel walk becomes the interpolator texture scan:
        accum[0/1] = rotated UV, base[0/1] = cos/sin step vector.
        BLADE_TILT folds into the pre-baked texture.
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

    in_ring = (r2 >= inner_r * inner_r) & (r2 <= outer_r * outer_r)

    phi = np.arctan2(dy, dx)

    # t=0 at inner edge, t=1 at outer — drives blade curvature
    t = np.clip((r - inner_r) / float(max(outer_r - inner_r, 1)), 0.0, 1.0)

    sector    = (2.0 * math.pi) / N_BLADES
    blade_pos = (phi - swirl * (blade_rot + BLADE_TILT * t)) % sector   # [0, sector)

    # edge width in radians ≈ BLADE_EDGE_PX screen pixels at this radius
    edge = BLADE_EDGE_PX / np.maximum(r, 1.0)

    canvas[y0:y1, x0:x1][in_ring & (blade_pos < edge)] = SECONDARY
