#include <iostream>
#include <vector>
#include "Tokenizer.h"
#include "Parser.h"
#include "ScopeAnalyzer.h"
using namespace std;


int main() {
    string file = "code.txt";

    vector<token> tokens = tokenize(file);
    //for (auto& i : tokens) cout << i << endl;

    Parser parser(tokens);
    parser.parse();

    ScopeAnalyzer analyzer;
    analyzer.analyse();

    return 0;
}
