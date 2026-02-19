# DISPLAY_BITMAP — 4-bit packed pixel buffer with 16-entry palette.
#
# Packing: 2 pixels per byte, low nibble = pixel 0, high nibble = pixel 1.
# pixel_index = y * WIDTH + x
# byte_index  = pixel_index // 2
# nibble      = pixel_index % 2   (0 = low, 1 = high)
#
# Palette layout (16 entries):
#   0  BG             3 base colors
#   1  SECONDARY
#   2  PRIMARY
#   3  RESERVED
#   4-6  BG → SECONDARY   (25 / 50 / 75 %)   ─┐
#   7-9  SECONDARY → PRIMARY                   ├─ AA gradients
#  10-12 PRIMARY → SECONDARY                   │
#  13-15 BG → PRIMARY  (spare)                ─┘

from dataclasses import dataclass, field

# ── display geometry ──────────────────────────────────────────────────────────
WIDTH  = 240
HEIGHT = 240

BITS_PER_COLOR  = 4
PIXELS_PER_BYTE = 2
PIXEL_MASK      = 0x0F
BITMAP_BYTES    = (HEIGHT * WIDTH) // PIXELS_PER_BYTE   # 28 800

# ── palette index constants ───────────────────────────────────────────────────
PI_BG         = 0
PI_SECONDARY  = 1
PI_PRIMARY    = 2
PI_RESERVED   = 3

PI_BG_SEC_1   = 4    # 25 % secondary
PI_BG_SEC_2   = 5    # 50 %
PI_BG_SEC_3   = 6    # 75 %

PI_SEC_PRI_1  = 7    # 25 % primary  (mostly secondary)
PI_SEC_PRI_2  = 8    # 50 %
PI_SEC_PRI_3  = 9    # 75 % primary

PI_PRI_SEC_1  = 10   # 25 % secondary (mostly primary)
PI_PRI_SEC_2  = 11   # 50 %
PI_PRI_SEC_3  = 12   # 75 % secondary

PI_BG_PRI_1   = 13   # spare — BG → PRIMARY gradient
PI_BG_PRI_2   = 14
PI_BG_PRI_3   = 15

# backward-compatible aliases (used by draw.py and legacy callers)
BG        = PI_BG
PRIMARY   = PI_PRIMARY
SECONDARY = PI_SECONDARY
RESERVED  = PI_RESERVED

# ── RGB565 colour constants ───────────────────────────────────────────────────
BLACK    = 0x0000
WHITE    = 0xFFFF
BLUE     = 0x001F
RED      = 0xF800
GREEN    = 0x07E0
MAGENTA  = 0xF81F
CYAN     = 0x7FFF
YELLOW   = 0xFFE0
BROWN    = 0xBC40
GRAY     = 0x8430
DARKCYAN = 0x05F7  # RGB565 (0, 47, 23) — ~75 % of CYAN brightness


# ── colour helpers ────────────────────────────────────────────────────────────
def rgb565_to_rgb888(c: int) -> tuple[int, int, int]:
    r = ((c >> 11) & 0x1F) << 3
    g = ((c >>  5) & 0x3F) << 2
    b =  (c        & 0x1F) << 3
    return (r, g, b)


def rgb888_to_rgb565(r: int, g: int, b: int) -> int:
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def _lerp_rgb565(a: int, b: int, t: float) -> int:
    """Blend two RGB565 colours in component space.  t=0 → a, t=1 → b."""
    r = round(((a >> 11) & 0x1F) * (1 - t) + ((b >> 11) & 0x1F) * t)
    g = round(((a >>  5) & 0x3F) * (1 - t) + ((b >>  5) & 0x3F) * t)
    bl= round( (a        & 0x1F) * (1 - t) + ( b        & 0x1F) * t)
    return (r << 11) | (g << 5) | bl


def build_palette(bg: int, primary: int,
                  secondary: int, reserved: int) -> list[int]:
    """Build the full 16-entry RGB565 palette from the 4 base colours."""
    p = [0] * 16
    p[PI_BG]       = bg
    p[PI_SECONDARY]= secondary
    p[PI_PRIMARY]  = primary
    p[PI_RESERVED] = reserved
    for i, t in enumerate((0.25, 0.50, 0.75)):
        p[PI_BG_SEC_1  + i] = _lerp_rgb565(bg,        secondary, t)
        p[PI_SEC_PRI_1 + i] = _lerp_rgb565(secondary, primary,   t)
        p[PI_PRI_SEC_1 + i] = _lerp_rgb565(primary,   secondary, t)
        p[PI_BG_PRI_1  + i] = _lerp_rgb565(bg,        primary,   t)
    return p


def dim_rgb565(color: int, brightness: int) -> int:
    """brightness 0–31."""
    brightness = min(brightness, 31)
    r = ((color >> 11) & 0x1F) * brightness // 31
    g = ((color >>  5) & 0x3F) * brightness // 31
    b =  (color        & 0x1F) * brightness // 31
    return (r << 11) | (g << 5) | b


# ── DisplayBitmap ─────────────────────────────────────────────────────────────
@dataclass
class DisplayBitmap:
    bg_color:        int = BLACK
    primary_color:   int = CYAN
    secondary_color: int = DARKCYAN
    reserved_color:  int = RED

    data: bytearray = field(default_factory=lambda: bytearray(BITMAP_BYTES))

    def color_lut(self) -> list[tuple[int, int, int]]:
        """16-entry RGB888 lookup table, derived from the 4 base colours."""
        pal = build_palette(self.bg_color, self.primary_color,
                            self.secondary_color, self.reserved_color)
        return [rgb565_to_rgb888(c) for c in pal]

    def clear(self):
        for i in range(BITMAP_BYTES):
            self.data[i] = 0

    def set_pixel(self, x: int, y: int, color_index: int):
        if x < 0 or x >= WIDTH or y < 0 or y >= HEIGHT:
            return
        pixel_index = y * WIDTH + x
        byte_index  = pixel_index >> 1          # // 2
        nibble      = pixel_index & 1           # 0 = low, 1 = high
        v           = color_index & PIXEL_MASK
        if nibble == 0:
            self.data[byte_index] = (self.data[byte_index] & 0xF0) | v
        else:
            self.data[byte_index] = (self.data[byte_index] & 0x0F) | (v << 4)

    def get_pixel_index(self, x: int, y: int) -> int:
        if x < 0 or x >= WIDTH or y < 0 or y >= HEIGHT:
            return PI_BG
        pixel_index = y * WIDTH + x
        byte_index  = pixel_index >> 1
        nibble      = pixel_index & 1
        b = self.data[byte_index]
        return b & 0x0F if nibble == 0 else (b >> 4) & 0x0F

    def set_from_array(self, array) -> None:
        """
        Replace entire bitmap from a HEIGHT×WIDTH uint8 colour-index (0-15) array.
        Fast path used by the numpy eye renderer; equivalent to the per-pixel
        PIO send loop write in C.
        """
        import numpy as np
        flat   = array.ravel().astype(np.uint8) & PIXEL_MASK
        packed = flat[0::2] | (flat[1::2] << 4)
        self.data[:] = packed.tobytes()
