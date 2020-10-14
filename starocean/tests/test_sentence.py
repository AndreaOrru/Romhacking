from copy import copy

from starocean.formatting import format, unformat

from .base import TestCase


class SentenceTestCase(TestCase):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.block = cls.rom.blocks[0x0B]
        cls.sentence = cls.block.sentences[0x24]

    def test_data(self):
        self.assertListEqual(self.sentence.data[0:2], [0x02, 0x02])
        self.assertListEqual(self.sentence.data[-3:], [0x01, 0xFF, 0xFF])

    def test_format_deformat(self):
        self.assertListEqual(unformat(format(self.sentence.data)), self.sentence.data)

    def test_text(self):
        self.assertIn("Man, it's quiet around here.", self.sentence.text)
        self.assertTrue(self.sentence.text.endswith("<WAIT><CLOSE>"))

    def test_setText(self):
        sentence = copy(self.sentence)
        new_text = "<DORN>:\nMan, it's LOUD around here.<WAIT><CLOSE>"

        sentence.setText(new_text)
        self.assertEqual(sentence.text, new_text)
