"""Unit tests for the OSC packet builder (no network, no AbleSet needed)."""
import unittest

from send_alt_toggle import UNITS, build_osc, send_toggle


class OscPacketTests(unittest.TestCase):
    def test_address_is_nul_terminated_and_padded(self):
        address = "/alt/guitar/toggle"
        packet = build_osc(address)
        self.assertTrue(packet.startswith(address.encode("utf-8") + b"\x00"))
        self.assertEqual(len(packet) % 4, 0)

    def test_empty_type_tag_is_padded(self):
        packet = build_osc("/alt/vocals/toggle")
        self.assertTrue(packet.endswith(b",\x00\x00\x00"))

    def test_all_units_build_valid_addresses(self):
        for unit in UNITS:
            packet = build_osc("/alt/{}/toggle".format(unit))
            self.assertTrue(packet.startswith(("/alt/" + unit).encode()))
            self.assertEqual(len(packet) % 4, 0)

    def test_invalid_address_rejected(self):
        with self.assertRaises(ValueError):
            build_osc("alt/guitar/toggle")

    def test_unknown_unit_rejected_before_sending(self):
        with self.assertRaises(ValueError):
            send_toggle("keyboard")


if __name__ == "__main__":
    unittest.main()
