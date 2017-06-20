#include <iostream>
#include <vector>
#include "args/args.hxx"
#include "block.hpp"
#include "rom.hpp"
#include "sentence.hpp"

using namespace std;


int main(int argc, char* argv[])
{
    // Define command line arguments.
    args::ArgumentParser parser("Translation tool for Seiken Densetsu 3.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});

    args::Group commands(parser, "AVAILABLE COMMANDS:", args::Group::Validators::Xor);
    args::Flag list(commands, "list", "List all the blocks", {'l', "list"});
    args::ValueFlag<string> extract(commands, "DUMP", "Extract all the blocks", {'e', "extract"});
    args::ValueFlag<string> insert(commands, "DUMP", "Reinsert all the blocks", {'i', "insert"});

    args::Group rom_group(parser, "", args::Group::Validators::All);
    args::Positional<string> rom(rom_group, "ROM", "Seiken Densetsu 3 ROM");


    // Parse arguments.
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


    // Execute the selected command.
    vector<Block> blocks;
    if (rom)
    {
        ROM::open(args::get(rom));
        blocks = Block::extractAll();
    }

    if (list)
    {
        for (auto& b: blocks)
            printf("%X\n", b.begin);
    }
    else if (extract)
    {
        auto sentences = Sentence::extractAll(blocks);
        FILE* dump = fopen(args::get(extract).c_str(), "w");

        for (auto& sentence: sentences)
            fprintf(dump, "[String $%X, $%X-%X]\n%s\n", sentence.block->begin,
                    sentence.begin, sentence.end, sentence.text.c_str());

        fclose(dump);
    }


    return 0;
}
