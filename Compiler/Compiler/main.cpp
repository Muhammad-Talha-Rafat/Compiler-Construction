#include <iostream>
#include <vector>
#include "Tokenizer.h"
#include "AST.h"
#include "Parser.h"
#include "ScopeAnalyzer.h"
using namespace std;



int main() {

    string file = "code.txt";

    vector<token> tokens = tokenize(file);
    //for (auto& i : tokens) cout << i << endl;

    Parser parser(tokens);
    Node* root = parser.parse();
    //root->print();

    ScopeAnalyzer analyser;
    analyser.analyse(root);
    analyser.print();

    return 0;
}
