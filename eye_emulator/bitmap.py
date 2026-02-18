# Exact Python mirror of DISPLAY_BITMAP from GUI_Paint.h.
# Packing: 2 bits/pixel, 4 pixels/byte, LSB = pixel 0.
# Color index layout: 00=BG  01=SECONDARY  10=RESERVED  11=PRIMARY

from dataclasses import dataclass, field

# ── display geometry ────────────────────────────────────────────────────────
WIDTH  = 240
HEIGHT = 240

BITS_PER_COLOR  = 2
PIXEL_MASK      = 0b11
PIXELS_PER_BYTE = 4
BITMAP_BYTES    = (HEIGHT * WIDTH) // PIXELS_PER_BYTE  # 14400

# ── 2-bit color indices ──────────────────────────────────────────────────────
BG        = 0b00
SECONDARY = 0b01
RESERVED  = 0b10
PRIMARY   = 0b11

# ── RGB565 palette constants (same names as GUI_Paint.h) ────────────────────
BLACK    = 0x0000
WHITE    = 0xFFFF
BLUE     = 0x001F
RED      = 0xF800
GREEN    = 0x07E0
MAGENTA  = 0xF81F
CYAN     = 0x7FFF
DARKCYAN = 0x05F7  # RGB565 (0, 47, 23) — ~75% of CYAN brightness
YELLOW   = 0xFFE0
BROWN    = 0xBC40
GRAY     = 0x8430


def rgb565_to_rgb888(c: int) -> tuple[int, int, int]:
    r = ((c >> 11) & 0x1F) << 3
    g = ((c >>  5) & 0x3F) << 2
    b =  (c        & 0x1F) << 3
    return (r, g, b)


def rgb888_to_rgb565(r: int, g: int, b: int) -> int:
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def dim_rgb565(color: int, brightness: int) -> int:
    """brightness 0-31, same formula as getDimmedColor in GUI_Paint.cpp"""
    brightness = min(brightness, 31)
    r = ((color >> 11) & 0x1F) * brightness // 31
    g = ((color >>  5) & 0x3F) * brightness // 31
    b =  (color        & 0x1F) * brightness // 31
    return (r << 11) | (g << 5) | b


@dataclass
class DisplayBitmap:
    bg_color:        int = BLACK
    primary_color:   int = CYAN
    secondary_color: int = DARKCYAN
    reserved_color:  int = RED

    # packed 2-bit pixel data, identical layout to C BitmapData[]
    data: bytearray = field(default_factory=lambda: bytearray(BITMAP_BYTES))

    # ── color lookup (index → RGB888), rebuilt on color change ──────────────
    def color_lut(self) -> list[tuple[int, int, int]]:
        return [
            rgb565_to_rgb888(self.bg_color),        # 00
            rgb565_to_rgb888(self.secondary_color),  # 01
            rgb565_to_rgb888(self.reserved_color),   # 10
            rgb565_to_rgb888(self.primary_color),    # 11
        ]

    def clear(self):
        for i in range(BITMAP_BYTES):
            self.data[i] = 0

    def set_pixel(self, x: int, y: int, color_index: int):
        if x < 0 or x >= WIDTH or y < 0 or y >= HEIGHT:
            return
        pixel_index = y * WIDTH + x
        byte_index  = pixel_index // PIXELS_PER_BYTE
        bit_offset  = (pixel_index %  PIXELS_PER_BYTE) * BITS_PER_COLOR
        self.data[byte_index] &= ~(PIXEL_MASK << bit_offset) & 0xFF
        self.data[byte_index] |=  (color_index & PIXEL_MASK) << bit_offset

    def set_from_array(self, array) -> None:
        """
        Replace entire bitmap from a HEIGHT×WIDTH uint8 color-index (0-3) array.
        This is the fast path used by the numpy-accelerated eye renderer.
        In C the equivalent is the interpolator texture-sample write loop.
        """
        import numpy as np
        flat = array.ravel().astype(np.uint8) & 0x03
        packed = (flat[0::4]        |
                 (flat[1::4] << 2)  |
                 (flat[2::4] << 4)  |
                 (flat[3::4] << 6)).astype(np.uint8)
        self.data[:] = packed.tobytes()

    def get_pixel_index(self, x: int, y: int) -> int:
        if x < 0 or x >= WIDTH or y < 0 or y >= HEIGHT:
            return BG
        pixel_index = y * WIDTH + x
        byte_index  = pixel_index // PIXELS_PER_BYTE
        bit_offset  = (pixel_index %  PIXELS_PER_BYTE) * BITS_PER_COLOR
        return (self.data[byte_index] >> bit_offset) & PIXEL_MASK
