"""
Eye emulator — two virtual 240×240 displays side by side.
Renders the packed DISPLAY_BITMAP struct via pygame + numpy.

Core1Thread listens on /tmp/robot_eyes.sock for UART-protocol packets
and puts commands on a queue — identical architecture to the real hardware.
Send commands with:  python send_cmd.py eyes --x 120 --y 80 --r 50

Local controls (keyboard override when no external commands arrive):
  Arrow keys  — move eye center
  [ / ]       — decrease / increase pupil radius (aperture)
  ESC / Q     — quit
"""

import queue
import sys

import numpy as np
import pygame

from anim     import EyeAnimator
from bitmap   import DisplayBitmap, WIDTH, HEIGHT
from commands import Cmd, EyeSettings
from core1    import Core1Thread
from draw     import clear_both
from eye      import (draw_camera_eyes,
                      OUTER_RADIUS, PUPIL_MIN_R, OUTER_RING_W, APERTURE_RING_W, DEFAULT_PUPIL_R)

# ── config ───────────────────────────────────────────────────────────────────
SCALE      = 2
GAP        = 16
FPS_TARGET = 60

PANEL_W = WIDTH  * SCALE
PANEL_H = HEIGHT * SCALE
WIN_W   = PANEL_W * 2 + GAP
WIN_H   = PANEL_H

PUPIL_MAX_R = OUTER_RADIUS - OUTER_RING_W - APERTURE_RING_W - 2


# ── bitmap → pygame surface (numpy fast path) ────────────────────────────────
def bitmap_to_surface(bm: DisplayBitmap) -> pygame.Surface:
    raw  = np.frombuffer(bm.data, dtype=np.uint8)
    idxs = np.empty(HEIGHT * WIDTH, dtype=np.uint8)
    idxs[0::4] =  raw        & 0x03
    idxs[1::4] = (raw >> 2)  & 0x03
    idxs[2::4] = (raw >> 4)  & 0x03
    idxs[3::4] = (raw >> 6)  & 0x03
    lut = np.array(bm.color_lut(), dtype=np.uint8)
    rgb = lut[idxs].reshape(HEIGHT, WIDTH, 3)
    if SCALE > 1:
        rgb = rgb.repeat(SCALE, axis=0).repeat(SCALE, axis=1)
    return pygame.surfarray.make_surface(rgb.swapaxes(0, 1))


# ── main ─────────────────────────────────────────────────────────────────────
def main():
    cmd_queue = queue.Queue()
    Core1Thread(cmd_queue).start()

    pygame.init()
    screen = pygame.display.set_mode((WIN_W, WIN_H))
    pygame.display.set_caption("Robot Eyes Emulator")
    clock = pygame.time.Clock()
    font  = pygame.font.SysFont("monospace", 12)

    # Single source of truth: EyeSettings defaults define the initial appearance.
    # To change startup colors, edit the EyeSettings dataclass in commands.py only.
    state = EyeSettings(radius=DEFAULT_PUPIL_R)

    def _make_bitmap(s: EyeSettings) -> DisplayBitmap:
        return DisplayBitmap(
            bg_color        = s.background_color,
            primary_color   = s.primary_color,
            secondary_color = s.secondary_color,
            reserved_color  = s.reserve_color,
        )

    right = _make_bitmap(state)
    left  = _make_bitmap(state)
    anim  = EyeAnimator(x=float(state.x), y=float(state.y), r=float(state.radius))

    running = True
    while running:
        # ── drain command queue (Core0 role) ─────────────────────────────────
        while not cmd_queue.empty():
            cmd, payload = cmd_queue.get_nowait()

            if cmd == Cmd.DRAW_EYES and payload is not None:
                state = payload
                right.bg_color        = state.background_color
                right.primary_color   = state.primary_color
                right.secondary_color = state.secondary_color
                right.reserved_color  = state.reserve_color
                left.bg_color         = state.background_color
                left.primary_color    = state.primary_color
                left.secondary_color  = state.secondary_color
                left.reserved_color   = state.reserve_color

            elif cmd in (Cmd.DRAW_INIT, Cmd.DRAW_LOADING, Cmd.DRAW_ERROR):
                clear_both(right, left)

        # ── keyboard ─────────────────────────────────────────────────────────
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key in (pygame.K_ESCAPE, pygame.K_q):
                    running = False
                elif event.key == pygame.K_LEFT:
                    state.x -= 4
                elif event.key == pygame.K_RIGHT:
                    state.x += 4
                elif event.key == pygame.K_UP:
                    state.y -= 4
                elif event.key == pygame.K_DOWN:
                    state.y += 4
                elif event.key == pygame.K_LEFTBRACKET:
                    state.radius = max(PUPIL_MIN_R, state.radius - 4)
                elif event.key == pygame.K_RIGHTBRACKET:
                    state.radius = min(PUPIL_MAX_R, state.radius + 4)

        # ── advance animator ─────────────────────────────────────────────────
        anim.step(state.x, state.y, state.radius, state.speed)
        cx, cy, cr = anim.as_int()

        # ── draw ─────────────────────────────────────────────────────────────
        draw_camera_eyes(right, left, cx, cy, cr)

        # ── render ───────────────────────────────────────────────────────────
        surf_r = bitmap_to_surface(right)
        surf_l = bitmap_to_surface(left)

        screen.fill((20, 20, 20))
        screen.blit(surf_r, (0,             0))
        screen.blit(surf_l, (PANEL_W + GAP, 0))

        label_r = font.render("RIGHT", True, (80, 80, 80))
        label_l = font.render("LEFT",  True, (80, 80, 80))
        screen.blit(label_r, (4,                  4))
        screen.blit(label_l, (PANEL_W + GAP + 4,  4))

        fps  = clock.get_fps()
        pct  = int(100 * (1.0 - (cr - PUPIL_MIN_R) / max(PUPIL_MAX_R - PUPIL_MIN_R, 1)))
        info = font.render(
            f"pos ({cx},{cy})  zoom {pct}%  r={cr}  spd={state.speed}  {fps:.0f} fps",
            True, (60, 60, 60),
        )
        screen.blit(info, (4, WIN_H - 18))

        pygame.display.flip()
        clock.tick(FPS_TARGET)

    pygame.quit()
    sys.exit(0)


if __name__ == "__main__":
    main()
