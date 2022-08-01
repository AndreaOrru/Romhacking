from copy import deepcopy
from collections import defaultdict
from typing import List, Tuple

from romhacking.utils import pairwise

N_SYMBOLS = 0x1000


def replacePair(data: List[int], pair: int, symbol: int):
    i = 0
    hi = pair & 0xFFFF
    low = pair >> 16
    while True:
        if i >= len(data) - 1:
            return data
        if (data[i] == hi) and (data[i + 1] == low):
            data[i] = symbol
            del data[i + 1]
        i += 1


def replacePairBlocks(blocks_data: List[List[List[int]]], pair: int, symbol: int):
    return [[replacePair(sentence_data, pair, symbol) for sentence_data in block_data] for block_data in blocks_data]


try:
    from romhacking.bytepair import bytepairCompress  # noqa
except ModuleNotFoundError:

    def bytepairCompress(
            blocks_data: List[List[List[int]]],
    ) -> Tuple[List[Tuple[int, int]], List[List[List[int]]]]:

        blocks_data = deepcopy(blocks_data)
        blocks_data = replacePairBlocks(blocks_data, 0x00FF00FF, 0x100)

        symbol = 0x101
        dictionary = []

        while symbol < N_SYMBOLS:
            occ = defaultdict(int)
            max_pair = 0
            max_occ = 0

            for block_data in blocks_data:
                for sentence in block_data:
                    for a, b in pairwise(sentence):
                        curr_pair = (b << 16) | a
                        occ[curr_pair] += 1
                        curr_occ = occ[curr_pair]
                        if curr_occ > max_occ:
                            max_occ = curr_occ
                            max_pair = curr_pair

            if max_occ < 4:
                break

            replacePairBlocks(blocks_data, max_pair, symbol)
            dictionary.append((max_pair & 0xFFFF, max_pair >> 16))
            symbol += 1

        return dictionary, blocks_data


def encode_12bit(strings: List[List[int]]) -> Tuple[List[int], List[int]]:
    indexes: List[int] = []
    output: List[int] = []

    index = len(strings) * 2 * 8
    shifted = False

    for string in strings:
        assert index <= 0xFFFF
        indexes.append(index)

        for c in string:
            assert c <= 0xFFF
            if shifted:
                output[-1] |= c >> 8
                output.append((c & 0xFF))
            else:
                output.append((c >> 4))
                output.append((c & 0xF) << 4)
            shifted = not shifted
            index += 12

    return indexes, output
