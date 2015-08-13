#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include "block.hpp"
#include "rom.hpp"
#include "sentence.hpp"

using namespace std;


int main(int argc, char* argv[])
{
    if (argc < 4)
        return 1;

    ROM::open(argv[2]);
    vector<Block> blocks = Block::extract_blocks();
    vector<Sentence> sentences = Sentence::extract_sentences(blocks);

    if (string(argv[1]) == "-e")
    {
        FILE* dump = fopen(argv[3], "w");

        for (auto& sentence: sentences)
            fprintf(dump, "[String $%X (%X), $%X-%X]\n%s\n", sentence.block->index, sentence.block->begin,
                    sentence.begin, sentence.end, sentence.text.c_str());
        fclose(dump);
    }

    else if (string(argv[1]) == "-i")
    {
        if (argc < 5)
            return 1;

        string tmp;
        ifstream dump(argv[3]);

        int i = 0;
        while (getline(dump, tmp, '\n'))
        {
            getline(dump, sentences[i].text, '[');
            sentences[i].text.resize(sentences[i].text.size() - 1);
            i++;
        }
        dump.close();

        Sentence::insert_sentences(sentences);
        Block::insert_blocks(blocks, argv[4]);
    }

    return 0;
}
