#!/usr/bin/env python3
"""
Send a command to the running emulator — simulates the robot main computer.

Usage:
  python send_cmd.py ping
  python send_cmd.py eyes --x 120 --y 80 --r 70
  python send_cmd.py eyes --x 60 --y 140 --r 40 --primary 0xF800
  python send_cmd.py init
"""

import argparse
import socket
import sys

from commands import (
    SOCKET_PATH, Cmd, Status,
    EyeSettings, build_packet, encode_draw_eyes,
)


def transact(packet: bytes) -> str:
    try:
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
            s.settimeout(2.0)
            s.connect(SOCKET_PATH)
            s.sendall(packet)
            resp = s.recv(256)
    except FileNotFoundError:
        return "error: emulator not running (socket not found)"
    except OSError as e:
        return f"error: {e}"

    if len(resp) < 4 or resp[0] != 0xAA:
        return f"bad response: {resp.hex()}"
    code = resp[1]
    try:
        return Status(code).name
    except ValueError:
        return f"unknown status ({code:#04x})"


def main():
    parser = argparse.ArgumentParser(description="Robot Eyes command sender")
    sub = parser.add_subparsers(dest='cmd', required=True)

    sub.add_parser('ping',    help='Check emulator is alive')
    sub.add_parser('init',    help='Trigger DRAW_INIT')
    sub.add_parser('loading', help='Trigger DRAW_LOADING')
    sub.add_parser('error',   help='Trigger DRAW_ERROR')

    eyes = sub.add_parser('eyes', help='Send DRAW_EYES')
    eyes.add_argument('--x',        type=int,                    default=120)
    eyes.add_argument('--y',        type=int,                    default=120)
    eyes.add_argument('--r',        type=int,                    default=60)
    eyes.add_argument('--speed',    type=int,                    default=5)
    eyes.add_argument('--primary',  type=lambda v: int(v, 0),   default=0x7FFF)  # CYAN
    eyes.add_argument('--secondary',type=lambda v: int(v, 0),   default=0x07E0)  # GREEN
    eyes.add_argument('--bg',       type=lambda v: int(v, 0),   default=0x0000)  # BLACK
    eyes.add_argument('--reserve',  type=lambda v: int(v, 0),   default=0xF800)  # RED

    args = parser.parse_args()

    if args.cmd == 'ping':
        pkt = build_packet(Cmd.PING)
    elif args.cmd == 'init':
        pkt = build_packet(Cmd.DRAW_INIT)
    elif args.cmd == 'loading':
        pkt = build_packet(Cmd.DRAW_LOADING)
    elif args.cmd == 'error':
        pkt = build_packet(Cmd.DRAW_ERROR)
    else:  # eyes
        s = EyeSettings(
            x=args.x, y=args.y, radius=args.r, speed=args.speed,
            primary_color=args.primary, secondary_color=args.secondary,
            background_color=args.bg, reserve_color=args.reserve,
        )
        pkt = encode_draw_eyes(s)

    print(transact(pkt))


if __name__ == '__main__':
    main()
