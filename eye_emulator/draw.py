# Drawing primitives — direct port of GUI_Paint.cpp.
# All integer arithmetic; no trig, no FP. Same constraints as RP2040 target.
# Each function operates on a single DisplayBitmap; callers manage clear().

from bitmap import DisplayBitmap, WIDTH, HEIGHT, BG, PRIMARY

R_EYE_OFFSET = 15   # right display: pupil center shifted right
L_EYE_OFFSET = 15   # left  display: pupil center shifted left


# ── line (Bresenham) ────────────────────────────────────────────────────────
def draw_line(bm: DisplayBitmap, x0: int, y0: int,
              x1: int, y1: int, color_index: int):
    dx =  abs(x1 - x0)
    dy = -abs(y1 - y0)
    sx = 1 if x0 < x1 else -1
    sy = 1 if y0 < y1 else -1
    err = dx + dy
    while True:
        bm.set_pixel(x0, y0, color_index)
        if x0 == x1 and y0 == y1:
            break
        e2 = 2 * err
        if e2 >= dy:
            if x0 == x1:
                break
            err += dy
            x0  += sx
        if e2 <= dx:
            if y0 == y1:
                break
            err += dx
            y0  += sy


# ── rectangle ───────────────────────────────────────────────────────────────
def draw_rectangle(bm: DisplayBitmap,
                   x0: int, y0: int, x1: int, y1: int,
                   color_index: int, filled: bool = True):
    x0 = max(x0, 0);      y0 = max(y0, 0)
    x1 = min(x1, WIDTH-1); y1 = min(y1, HEIGHT-1)
    if filled:
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                bm.set_pixel(x, y, color_index)
    else:
        draw_line(bm, x0, y0, x1, y0, color_index)
        draw_line(bm, x0, y0, x0, y1, color_index)
        draw_line(bm, x1, y1, x1, y0, color_index)
        draw_line(bm, x1, y1, x0, y1, color_index)


# ── circle ───────────────────────────────────────────────────────────────────
# Faithful port of Bitmap_DrawCircle from GUI_Paint.cpp.
# Outline threshold: -Radius*10 < dx²+dy²-r² <= 0  (thick, radius-dependent).
def draw_circle(bm: DisplayBitmap, cx: int, cy: int,
                radius: int, color_index: int, filled: bool = True):
    if radius <= 0:
        return
    x0 = max(cx - radius, 0)
    y0 = max(cy - radius, 0)
    x1 = min(cx + radius, WIDTH  - 1)
    y1 = min(cy + radius, HEIGHT - 1)
    r2 = radius * radius

    if filled:
        for y in range(y0, y1):
            for x in range(x0, x1):
                dx = cx - x
                dy = cy - y
                if dx * dx + dy * dy <= r2:
                    bm.set_pixel(x, y, color_index)
    else:
        thresh = radius * 10
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                dx   = cx - x
                dy   = cy - y
                offs = dx * dx + dy * dy - r2
                if -thresh < offs <= 0:
                    bm.set_pixel(x, y, color_index)


# ── eye helpers ──────────────────────────────────────────────────────────────
def clear_both(right: DisplayBitmap, left: DisplayBitmap):
    right.clear()
    left.clear()


def draw_eyes(right: DisplayBitmap, left: DisplayBitmap,
              x: int, y: int, radius: int, color_index: int = PRIMARY):
    """Draw one filled circle per display with the hardware side-offset applied."""
    draw_circle(right, x + R_EYE_OFFSET, y, radius, color_index, filled=True)
    draw_circle(left,  x - L_EYE_OFFSET, y, radius, color_index, filled=True)
