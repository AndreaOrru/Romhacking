from starocean.block import BlockType

from .base import TestCase


class ROMTestCase(TestCase):
    def test_title(self):
        self.assertTrue(self.rom.title.startswith("Star Ocean"))

    def test_address_translation(self):
        self.assertEqual(self.rom.readWord(0xE6D49C), 0x0510)

    def test_blocks(self):
        block = self.rom.blocks[0x0B]
        self.assertEqual(block.start, 0xE6D493)
        self.assertEqual(block.type, BlockType.TEXT)
        self.assertEqual(block.index, 0x0B)
