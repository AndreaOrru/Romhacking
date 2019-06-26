from unittest import TestCase

from romhacking.utils import convert16to8


class UtilsTestCase(TestCase):
    def test_convert16to8(self):
        data = convert16to8([0x1234, 0x5678, 0x9ABC])
        self.assertListEqual(data, [0x34, 0x12, 0x78, 0x56, 0xBC, 0x9A])
