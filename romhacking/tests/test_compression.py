from unittest import TestCase

from romhacking.compression import (
    bytepairCompress,
    encode_12bit,
    replacePair,
    replacePairBlocks,
)


class CompressionTestCase(TestCase):
    def test_encode_12bit(self):
        strings = [[0x123, 0x456], [0x789, 0xABC]]
        indexes, data = encode_12bit(strings)

        self.assertEqual(len(indexes), 2)
        base_index = len(indexes) * 2 * 8

        self.assertEqual(indexes[0], base_index + (0 * 12))
        self.assertEqual(indexes[1], base_index + (2 * 12))

        self.assertListEqual(data, [0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC])

    def test_replacePair(self):
        data = [0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC]
        replacePair(data, 0x0034_0012, 0x100)
        replacePair(data, 0x0078_0056, 0x101)
        replacePair(data, 0x00BC_009A, 0x102)
        replacePair(data, 0x0101_0100, 0x103)
        replacePair(data, 0x0102_0103, 0x104)
        self.assertListEqual(data, [0x104])

    def test_replacePairBlocks(self):
        data = [
            [
                [0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC],
                [0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC],
            ],
            [
                [0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC],
                [0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC],
            ],
        ]
        replacePairBlocks(data, 0x0034_0012, 0x100)
        replacePairBlocks(data, 0x0078_0056, 0x101)
        replacePairBlocks(data, 0x00BC_009A, 0x102)
        replacePairBlocks(data, 0x0101_0100, 0x103)
        replacePairBlocks(data, 0x0102_0103, 0x104)
        self.assertListEqual(
            data,
            [
                [[0x104], [0x104]],
                [[0x104], [0x104]],
            ],
        )

    def test_bytepairCompress(self):
        string1 = "low low low low entropy entropy entropy entropy"
        string2 = "entropy entropy entropy entropy low low low low"
        data = [
            [
                [ord(x) for x in string1],
                [ord(x) for x in string2],
            ]
        ]

        dictionary, data = bytepairCompress(data)
        for compressed_string in data[0]:
            self.assertTrue(all(x > 0xFF for x in compressed_string))
