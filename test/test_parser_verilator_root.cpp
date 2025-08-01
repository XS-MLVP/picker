#include "parser/verilator_root.hpp"

using namespace std;

bool picker::is_debug = 1;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input.cpp>" << endl;
        return 1;
    }

    auto declarations = picker::parser::verilator::readVarDeclarations(argv[1]);
    auto variables    = picker::parser::verilator::processDeclarations(declarations);
    picker::parser::outputYAML(variables);

    return 0;
}