from .base import TestCase


class SentenceTestCase(TestCase):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.sentence = cls.rom.sentences[421]
        cls.ascii_sentence = cls.rom.sentences[422]

    def test_is_ascii(self):
        self.assertFalse(self.sentence.is_ascii)
        self.assertTrue(self.ascii_sentence.is_ascii)

    def test_data(self):
        self.assertEqual(self.sentence.data, "Podunk, U.S.A.\x00")
        self.assertEqual(self.ascii_sentence.data, "\x87Fall, 1965\x00")

    def test_text(self):
        self.assertEqual(self.sentence.text, "Podunk, U.S.A.<End>")
        self.assertEqual(self.ascii_sentence.text, "<$87>Fall, 1965<End>")

        complex_sentence = self.rom.sentences[565]
        self.assertIn("You need the Drain Formula!", complex_sentence.text)

    def test_expandDTE(self):
        self.assertEqual(self.sentence._expandDTE(0x87), "s ")

    def test_expandMTE_8(self):
        self.assertEqual(self.sentence._expandMTE_8(0xD1), "ingredients")

    def test_expandMTE_16(self):
        self.assertEqual(self.sentence._expandMTE_16(0x11), "aren't")
