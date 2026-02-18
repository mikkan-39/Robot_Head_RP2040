# Ease-out animator for eye position and radius.
#
# Algorithm: exponential decay — each frame, close a fraction of the
# remaining distance.  The fraction is derived from EyeSettings.speed
# as a power-of-two divisor so the C port can use integer right-shift:
#
#   C:      cur_x += (target_x - cur_x) >> shift
#   Python: cur_x += (target_x - cur_x) / (1 << shift)
#
# speed → shift mapping (tweak these constants; same table goes in C):
#   speed  1 →  shift 5  → factor  3%   (~2 s to 95%)
#   speed  2 →  shift 4  → factor  6%   (~1 s to 95%)
#   speed  4 →  shift 3  → factor 12%   (~0.5 s to 95%)
#   speed  8 →  shift 2  → factor 25%   (~0.25 s to 95%)
#   speed 16 →  shift 1  → factor 50%   (4 frames to 95%)
#   speed 32 →  snap immediately
#
# "95% there" times assume 60 fps.

from dataclasses import dataclass

SNAP_THRESHOLD = 0.5   # pixels — below this, hard-snap to target


def _speed_to_shift(speed: int) -> int:
    """Map speed (1-255) to a right-shift amount (0 = snap)."""
    if speed >= 32: return 0
    if speed >= 16: return 1
    if speed >=  8: return 2
    if speed >=  4: return 3
    if speed >=  2: return 4
    return 5


@dataclass
class EyeAnimator:
    x: float = 120.0
    y: float = 120.0
    r: float =  60.0

    def step(self, target_x: int, target_y: int, target_r: int, speed: int):
        shift = _speed_to_shift(speed)

        if shift == 0:
            self.x = float(target_x)
            self.y = float(target_y)
            self.r = float(target_r)
            return

        divisor = 1 << shift          # integer divisor — maps to >> in C

        dx = target_x - self.x
        dy = target_y - self.y
        dr = target_r - self.r

        self.x += dx / divisor
        self.y += dy / divisor
        self.r += dr / divisor

        # hard-snap when sub-pixel — avoids infinite approach
        if abs(target_x - self.x) < SNAP_THRESHOLD: self.x = float(target_x)
        if abs(target_y - self.y) < SNAP_THRESHOLD: self.y = float(target_y)
        if abs(target_r - self.r) < SNAP_THRESHOLD: self.r = float(target_r)

    def as_int(self) -> tuple[int, int, int]:
        return round(self.x), round(self.y), round(self.r)

    def at_target(self, target_x: int, target_y: int, target_r: int) -> bool:
        return (self.x == target_x and
                self.y == target_y and
                self.r == target_r)
