#include <fstream>
#include <iostream>
#include <vector>
#include "args/args.hxx"
#include "block.hpp"
#include "rom.hpp"
#include "sentence.hpp"

using namespace std;


int main(int argc, char* argv[])
{
    ////
    // Define command line arguments.
    //
    args::ArgumentParser parser("Translation tool for Seiken Densetsu 3.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});

    args::Group commands(parser, "AVAILABLE COMMANDS:", args::Group::Validators::Xor);
    args::ValueFlag<string> extract(commands, "DUMP", "Extract all the blocks", {'e', "extract"});
    args::ValueFlag<string> insert(commands, "DUMP", "Reinsert all the blocks", {'i', "insert"});

    args::Group rom_group(parser, "", args::Group::Validators::All);
    args::Positional<string> rom(rom_group, "ROM", "Seiken Densetsu 3 ROM");


    ////
    // Parse arguments.
    //
    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
        std::cout << parser;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (args::ValidationError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }


    ////
    // Execute the selected command.
    //
    vector<Block> blocks;
    map<pair<int,int>, Sentence*> sentences;
    if (rom)
    {
        ROM::open(args::get(rom));
        blocks = Block::extractAll();
        sentences = Sentence::extractAll(blocks);
    }

    if (extract)
    {
        // Open the dump file for writing.
        FILE* dump = fopen(args::get(extract).c_str(), "w");

        // Iterate through the sentences and write them on the file.
        for (auto& pair: sentences)
        {
            auto sentence = pair.second;
            fprintf(dump, "[Block $%X, String $%X-%X]\n%s\n", sentence->block->begin,
                    sentence->begin, sentence->end, sentence->text.c_str());
        }

        fclose(dump);
    }
    else if (insert)
    {
        // Open the dump file for reading.
        ifstream dump(args::get(insert).c_str());

        // Find the first sentence.
        string header;
        getline(dump, header, '[');

        // Iterate through the sentences.
        while (getline(dump, header, '\n'))
        {
            // Parse the sentence header.
            int block_address, sentence_begin, sentence_end;
            sscanf(header.c_str(), "Block $%X, String $%X-%X]",
                   &block_address, &sentence_begin, &sentence_end);

            // Get the associated sentence object.
            auto sentence = sentences[make_pair(block_address, sentence_begin)];
            // Get the new text of the sentence and save it in the object.
            getline(dump, sentence->text, '[');

            // Remove trailing newline.
            if (sentence->text.back() == '\n')
                sentence->text.pop_back();
        }

        // Reinsert everything into the ROM.
        Sentence::insertAll(sentences);
        Block::insertAll(blocks);
        ROM::save();
    }


    return 0;
}
