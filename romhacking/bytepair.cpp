#include <array>
#include <boost/python.hpp>
#include <list>
#include <vector>

namespace py = boost::python;
using namespace std;

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef vector<vector<list<u16>>> BlocksData;

const size_t N_SYMBOLS = 0x1000;

BlocksData fromPython(py::list &blocks) {
  BlocksData blocks_data;

  for (int i = 0; i < py::len(blocks); i++) {
    py::list block = py::extract<py::list>(blocks[i]);
    auto &block_data = blocks_data.emplace_back();

    for (int j = 0; j < py::len(block); j++) {
      py::list sentence = py::extract<py::list>(block[j]);
      auto &sentence_data = block_data.emplace_back();

      for (int k = 0; k < py::len(sentence); k++) {
        u8 byte = py::extract<int>(sentence[k]);
        sentence_data.push_back(byte);
      }
    }
  }
  return blocks_data;
}

py::list toPython(BlocksData &blocks_data) {
  py::list blocks;

  for (auto &block_data : blocks_data) {
    py::list block;
    for (auto &sentence_data : block_data) {
      py::list sentence;
      for (auto byte : sentence_data) {
        sentence.append(byte);
      }
      block.append(sentence);
    }
    blocks.append(block);
  }
  return blocks;
}

void replacePair(list<u16> &data, u32 pair, u16 symbol) {
  for (auto it = data.begin(); it != prev(data.end()); it++) {
    if (*it == (pair & 0xFFFF) and *next(it) == (pair >> 16)) {
      *it = symbol;
      data.erase(next(it));
    }
  }
}

void replacePairBlocks(BlocksData &blocks_data, u32 pair, u16 symbol) {
  for (auto &block_data : blocks_data) {
    for (auto &sentence_data : block_data) {
      replacePair(sentence_data, pair, symbol);
    }
  }
}

py::tuple bytepairCompress(py::list &blocks) {
  auto blocks_data = fromPython(blocks);
  auto occ = new array<array<int, N_SYMBOLS>, N_SYMBOLS>;
  py::list dictionary;

  replacePairBlocks(blocks_data, 0x00FF00FF, 0x100);

  for (u16 symbol = 0x101; symbol < N_SYMBOLS; symbol++) {
    u32 max_pair = 0;
    int max_occ = 0;
    for (auto &arr : *occ) {
      arr.fill(0);
    }

    for (auto &block_data : blocks_data) {
      for (auto &sentence : block_data) {
        for (auto it = sentence.begin(); it != prev(sentence.end()); it++) {
          int curr_occ = ++(*occ)[*it][*next(it)];
          if (curr_occ > max_occ) {
            max_occ = curr_occ;
            max_pair = (*next(it) << 16) | *it;
          }
        }
      }
    }
    if (max_occ < 4) {
      break;
    }

    replacePairBlocks(blocks_data, max_pair, symbol);
    dictionary.append(py::make_tuple(max_pair & 0xFFFF, max_pair >> 16));
  }

  delete occ;
  return py::make_tuple(dictionary, toPython(blocks_data));
}

BOOST_PYTHON_MODULE(bytepair) { def("bytepairCompress", bytepairCompress); }
