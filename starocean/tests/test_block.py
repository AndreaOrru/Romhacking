from starocean.block import BlockType

from .base import TestCase


class BlockTestCase(TestCase):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.block = cls.rom.blocks[0x0B]

    def test_type(self):
        self.assertEqual(self.block.type, BlockType.TEXT)

    def test_size(self):
        self.assertEqual(self.block._indexes_size, len(self.block.indexes) * 2)
        self.assertEqual(self.block.size, 0x822)
        self.assertEqual(
            self.block.size,
            (
                len(self.block.header)
                + len(self.block.indexes) * 2
                + len(self.block.content)
            ),
        )

    def test_end(self):
        self.assertEqual(self.block.end, self.rom.blocks[0x0D].start)

    def test_data(self):
        self.assertEqual(len(self.block.data), self.block.size)

    def test_decompressRange(self):
        data = self.block._decompressRange(self.block.indexes[0], self.block.indexes[1])
        self.assertEqual(data[0], 0x89)
        self.assertEqual(data[-1], 0x89)
        self.assertEqual(len(data), 0xD)
