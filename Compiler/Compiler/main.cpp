#include <vector>
#include "Tokenizer.h"
#include "Structs.h"
#include "Parser.h"
#include "ScopeAnalyser.h"
#include "TypeChecker.h"
using namespace std;



int main() {

    string file = "code.txt";

    vector<token> tokens = tokenize(file);
    //for (auto& i : tokens) cout << i << endl;

    Parser parser(tokens);
    Node* root = parser.parse();

    if (!hasParseError(root)) {
        ScopeAnalyser analyser(root);
        Scope* scopeTable = analyser.getScopeTable();

        if (!hasScopeWarnings(analyser)) {
            TypeChecker checker(root, scopeTable);
            checker.check();
            root->print();
        }
        else scopeTable->print();
    }
    else root->print();

    return 0;
}