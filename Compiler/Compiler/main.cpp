#include <iostream>
#include <vector>
#include "Tokenizer.h"
#include "Structs.h"
#include "Parser.h"
#include "ScopeAnalyser.h"
using namespace std;


bool hasParsingError(Node* root) {
    Node* temp = root;
    while (temp && !temp->children.empty()) {
        temp = temp->children.back();
    }
    if (temp && temp->type == "error") {
        cout << "[\033[1;31mCritical\033[0m] Unfortunately an error has been encountered during parsing"
            << "\ncannot do scope analysis, print the AST to further information\n";
        return true;
    }
    else return false;
}

int main() {

    string file = "code.txt";

    vector<token> tokens = tokenize(file);
    //for (auto& i : tokens) cout << i << endl;

    Parser parser(tokens);
    Node* root = parser.parse();
    //root->print();

    if (!hasParsingError(root)) {
        ScopeAnalyser analyser(root);
        Scope* scope = analyser.getScopeTable();
        scope->print();
    }

    return 0;
}
