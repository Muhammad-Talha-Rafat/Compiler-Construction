#include <iostream>
#include <vector>
#include "Tokenizer.h"
#include "Parser.h"
using namespace std;

int main() {
    string file = "code.txt";

    vector<token> tokens = tokenize(file);
    //for (auto& i : tokens) i.print();

    Parser parser = Parser(tokens);

    cout << parser.isHeaders();

    return 0;
}