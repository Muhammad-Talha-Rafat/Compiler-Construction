#include <vector>
#include "Tokenizer.h"
#include "Structs.h"
#include "Parser.h"
#include "ScopeAnalyser.h"
using namespace std;




int main() {

    string file = "code.txt";

    vector<token> tokens = tokenize(file);
    //for (auto& i : tokens) cout << i << endl;

    Parser parser(tokens);
    Node* root = parser.parse();
    //root->print();

    if (!hasParseError(root)) {
        ScopeAnalyser analyser(root);
        Scope* scope = analyser.getScopeTable();
        scope->print();
    }

    return 0;
}
