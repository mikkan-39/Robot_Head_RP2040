# Protocol constants — mirrors MulticoreUtils.h + UART_Utils.h + Core1.cpp.
# Shared by core1.py (listener) and send_cmd.py (sender).

import struct
from dataclasses import dataclass
from enum import IntEnum

from bitmap import BLACK, CYAN, GREEN, RED

START_BYTE  = 0xAA
SOCKET_PATH = '/tmp/robot_eyes.sock'


class Cmd(IntEnum):
    PING         = 0x01
    DRAW_INIT    = 0x02
    DRAW_LOADING = 0x03
    DRAW_ERROR   = 0x04
    DRAW_EYES    = 0x05


class Status(IntEnum):
    OK                 = 0x00
    INVALID_COMMAND    = 0x01
    INVALID_ARG        = 0x02
    WRONG_CHECKSUM     = 0x03
    INCOMPLETE_REQUEST = 0x04


@dataclass
class EyeSettings:
    """Mirrors the EyeSettings struct from MulticoreUtils.h."""
    x:                int = 120
    y:                int = 120
    radius:           int = 60
    speed:            int = 5
    background_color: int = BLACK
    primary_color:    int = CYAN
    secondary_color:  int = GREEN
    reserve_color:    int = RED


# ── CRC-8 Dallas/Maxim (poly 0x8C) — same algorithm as UART_Utils.cpp ───────
def crc8(data: bytes) -> int:
    crc = 0
    for byte in data:
        extract = byte
        for _ in range(8):
            s = (crc ^ extract) & 0x01
            crc >>= 1
            if s:
                crc ^= 0x8C
            extract >>= 1
    return crc


def validate_checksum(data: bytes) -> bool:
    return len(data) >= 4 and crc8(data[:-1]) == data[-1]


# ── packet builder ────────────────────────────────────────────────────────────
def build_packet(command: int, payload: bytes = b'') -> bytes:
    header = bytes([START_BYTE, command, len(payload)]) + payload
    return header + bytes([crc8(header)])


# ── DRAW_EYES TLV encoder ─────────────────────────────────────────────────────
def encode_draw_eyes(s: EyeSettings) -> bytes:
    tlv  = bytes([0x01, 1, s.x      & 0xFF])
    tlv += bytes([0x02, 1, s.y      & 0xFF])
    tlv += bytes([0x03, 1, s.radius & 0xFF])
    tlv += bytes([0x04, 1, s.speed  & 0xFF])
    tlv += bytes([0x05, 2]) + struct.pack('<H', s.background_color)
    tlv += bytes([0x06, 2]) + struct.pack('<H', s.primary_color)
    tlv += bytes([0x07, 2]) + struct.pack('<H', s.secondary_color)
    tlv += bytes([0x08, 2]) + struct.pack('<H', s.reserve_color)
    return build_packet(Cmd.DRAW_EYES, tlv)


# ── DRAW_EYES TLV decoder (mirrors handle_draw_eyes_command in Core1.cpp) ────
def decode_draw_eyes(payload: bytes, s: EyeSettings):
    i = 0
    while i + 2 <= len(payload):
        tag       = payload[i]
        field_len = payload[i + 1]
        i += 2
        if i + field_len > len(payload):
            break
        val = payload[i:i + field_len]
        if   tag == 0x01 and field_len == 1: s.x                = val[0]
        elif tag == 0x02 and field_len == 1: s.y                = val[0]
        elif tag == 0x03 and field_len == 1: s.radius           = val[0]
        elif tag == 0x04 and field_len == 1: s.speed            = val[0]
        elif tag == 0x05 and field_len == 2: s.background_color = struct.unpack_from('<H', val)[0]
        elif tag == 0x06 and field_len == 2: s.primary_color    = struct.unpack_from('<H', val)[0]
        elif tag == 0x07 and field_len == 2: s.secondary_color  = struct.unpack_from('<H', val)[0]
        elif tag == 0x08 and field_len == 2: s.reserve_color    = struct.unpack_from('<H', val)[0]
        i += field_len
