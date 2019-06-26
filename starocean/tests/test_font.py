from .base import TestCase


class FontTestCase(TestCase):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.font = cls.rom.font

    def test_widths(self):
        self.assertEqual(self.font.widths[0], 3)  # Space.
        self.assertEqual(self.font.widths[1], 7)  # 0.
        self.assertEqual(self.font.widths[2], 5)  # 1.

    def test_vwf(self):
        vwf = self.font.generateVWF([0x01, 0x02, 0x03, 0x04])

        self.assertEqual(
            self.font.displayChar(vwf[0]), '\n'.join([
                '                ',
                '                ',
                '                ',
                '  ####    ##  ##',
                ' ##  ##  ### #  ',
                ' ##  ## ####    ',
                ' ## ###   ##    ',
                ' ### ##   ##    ',
                ' ##  ##   ##   #',
                ' ##  ##   ##  ##',
                ' ##  ##   ## ## ',
                '  ####    ## ###',
                '                ',
                '                ',
                '                ',
                '                ',
            ]))

        self.assertEqual(
            self.font.displayChar(vwf[1]), '\n'.join([
                '                ',
                '                ',
                '                ',
                '##  ######      ',
                ' ##    ##       ',
                ' ##   ##        ',
                ' ##  ####       ',
                '##      ##      ',
                '#       ##      ',
                '        ##      ',
                '    #   ##      ',
                '###  ####       ',
                '                ',
                '                ',
                '                ',
                '                ',
            ]))
