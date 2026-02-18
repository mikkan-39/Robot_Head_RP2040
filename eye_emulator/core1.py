# Emulates Core1's UART listener (Core1.cpp).
# Runs in a daemon thread; accepts bytes on a Unix socket, feeds them through
# the same byte-by-byte state machine as on_serial_rx(), puts parsed
# (Cmd, EyeSettings|None) tuples on a queue.Queue for the main thread.
# Sends status response packets back through the socket (mirrors send_status()).

import copy
import os
import queue
import socket
import threading

from commands import (
    START_BYTE, Cmd, Status, EyeSettings,
    validate_checksum, build_packet, decode_draw_eyes, SOCKET_PATH,
)


def _dispatch(buf: bytearray, cmd_queue: queue.Queue,
              current: EyeSettings, lock: threading.Lock) -> bytes:
    """
    Mirrors parse_command() in Core1.cpp.

    current is the persistent EyeSettings — exactly like desired_settings in C.
    Only the TLV fields present in the packet are updated; everything else
    keeps its previous value.  A copy goes on the queue so the draw thread
    always sees a consistent snapshot.
    """
    command = buf[1]
    length  = buf[2]
    payload = bytes(buf[3:3 + length])

    if command == Cmd.PING:
        return build_packet(Status.OK)

    if command in (Cmd.DRAW_INIT, Cmd.DRAW_LOADING, Cmd.DRAW_ERROR):
        cmd_queue.put((command, None))
        return build_packet(Status.OK)

    if command == Cmd.DRAW_EYES:
        with lock:
            decode_draw_eyes(payload, current)          # update only sent fields
            cmd_queue.put((command, copy.copy(current))) # snapshot for Core0
        return build_packet(Status.OK)

    return build_packet(Status.INVALID_COMMAND)


def _handle_connection(conn: socket.socket, cmd_queue: queue.Queue,
                       current: EyeSettings, lock: threading.Lock):
    """Feeds received bytes through on_serial_rx() state machine."""
    buf          = bytearray()
    syncing      = True
    expected_len = 0

    while True:
        try:
            chunk = conn.recv(256)
        except OSError:
            break
        if not chunk:
            break

        for byte in chunk:
            if syncing:
                if byte == START_BYTE:
                    buf.clear()
                    buf.append(byte)
                    syncing = False
                continue

            buf.append(byte)

            if len(buf) == 3:
                expected_len = buf[2]
            elif len(buf) == 3 + expected_len + 1:
                if validate_checksum(bytes(buf)):
                    response = _dispatch(buf, cmd_queue, current, lock)
                else:
                    response = build_packet(Status.WRONG_CHECKSUM)
                try:
                    conn.sendall(response)
                except OSError:
                    pass
                syncing = True

            if len(buf) > 256:
                syncing = True


class Core1Thread(threading.Thread):
    """Daemon thread — exits when main thread exits."""

    def __init__(self, cmd_queue: queue.Queue):
        super().__init__(daemon=True, name='Core1')
        self.cmd_queue = cmd_queue
        # Persistent state — mirrors desired_settings global in C.
        # Initialised from EyeSettings defaults (the ONE place colors are defined).
        self._current  = EyeSettings()
        self._lock     = threading.Lock()

    def run(self):
        if os.path.exists(SOCKET_PATH):
            os.unlink(SOCKET_PATH)

        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as srv:
            srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            srv.bind(SOCKET_PATH)
            srv.listen(4)
            while True:
                try:
                    conn, _ = srv.accept()
                except OSError:
                    break
                t = threading.Thread(
                    target=_handle_connection,
                    args=(conn, self.cmd_queue, self._current, self._lock),
                    daemon=True,
                )
                t.start()
