#!/usr/bin/env python3
"""Send one ALT-toggle OSC message to AbleSet (build-time helper, no AI needed).

The show's AbleSet Project Script listens for /alt/<unit>/toggle on UDP 39051.
This script only sends that message; the WING OSC stays in AbleSet.
"""
import argparse
import socket
import struct

UNITS = ("guitar", "bass", "flute", "vocals", "drums")
DEFAULT_HOST = "192.168.1.2"  # change per show machine
DEFAULT_PORT = 39051


def _pad(data: bytes) -> bytes:
    """Pad an OSC string/type-tag to a 4-byte boundary with NUL bytes."""
    return data + b"\x00" * ((4 - len(data) % 4) % 4)


def build_osc(address: str) -> bytes:
    """Build an OSC message with no arguments (address + empty type tag)."""
    if not address.startswith("/"):
        raise ValueError("OSC address must start with '/'")
    return _pad(address.encode("utf-8")) + _pad(b",")


def send_toggle(unit: str, host: str = DEFAULT_HOST, port: int = DEFAULT_PORT) -> bytes:
    """Send /alt/<unit>/toggle and return the bytes that were sent."""
    if unit not in UNITS:
        raise ValueError("unknown unit {!r}; expected one of {}".format(unit, ", ".join(UNITS)))
    packet = build_osc("/alt/{}/toggle".format(unit))
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.sendto(packet, (host, port))
    return packet


def main() -> None:
    parser = argparse.ArgumentParser(description="Send an ALT toggle to AbleSet")
    parser.add_argument("unit", choices=UNITS)
    parser.add_argument("--host", default=DEFAULT_HOST)
    parser.add_argument("--port", type=int, default=DEFAULT_PORT)
    args = parser.parse_args()
    data = send_toggle(args.unit, args.host, args.port)
    print("sent /alt/{}/toggle ({} bytes) to {}:{}".format(
        args.unit, len(data), args.host, args.port))


if __name__ == "__main__":
    main()
