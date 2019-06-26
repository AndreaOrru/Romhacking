from unittest import TestCase

from romhacking.malloc import Heap
from romhacking.rom import ROM

from .test_rom import assemble


class MallocTestCase(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.rom = ROM(assemble('hirom.asm'))

    def setUp(self):
        self.heap = Heap(self.rom)

    def test_addFreeArea(self):
        self.heap.addFreeArea(0xFDF000, 0xFE1000)
        self.heap.addFreeArea(0xFE1000, 0xFE2000)
        self.heap.addFreeArea(0xFE2000, 0xFF3000)
        self.assertListEqual(list(self.heap.areas), [
            (0xFDF000, 0xFE0000),
            (0xFE0000, 0xFF0000),
            (0xFF0000, 0xFF3000),
        ])

    def test_allocate(self):
        self.heap.addFreeArea(0xFDF000, 0xFF3000)

        block1 = self.heap.allocate(0x2000)
        self.assertEqual(block1, 0xFE0000)

        block2 = self.heap.allocate(0x1000)
        self.assertEqual(block2, 0xFDF000)

    def test_intersectsAreas(self):
        self.heap.addFreeArea(0xFE0000, 0xFE3000)
        self.heap.addFreeArea(0xFE7000, 0xFEA000)

        self.assertTrue(self.heap._intersectsAreas(0xFE2000, 0xFE8000))
        self.assertFalse(self.heap._intersectsAreas(0xFF0000, 0xFF2000))

    def test_splitArea(self):
        self.heap.addFreeArea(0xFE0000, 0xFE5000)

        busy_part = self.heap._splitArea(0, 0x2000)
        self.assertTupleEqual(busy_part, (0xFE0000, 0xFE2000))

        free_part = (0xFE2000, 0xFE5000)
        self.assertEqual(len(self.heap.areas), 1)
        self.assertEqual(self.heap.areas[0], free_part)
