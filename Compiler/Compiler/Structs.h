#pragma once
#include <string>
#include <vector>
using namespace std;



struct Node {

    string type;
    string value;
    vector<Node*> children;

    Node(const string& _type, const string& _value = "")
        : type(_type), value(_value) {}

    Node(const token& _token)
        : type(_token.type), value(_token.value) {}


    void print(int depth = 0) const {

        for (int i = 0; i < depth; i++)
            cout << "   ";

        if (type == "error") {
            cout << value << endl;
            exit(1);
        }

        if (!this->children.empty())
            cout << "\033[1;37m" << type << "\033[0m";
        else cout << "\033[1;33m" << type << "\033[0m";

        if (!value.empty())
            cout << ": " << value;
        cout << endl;

        for (Node* child : children)
            if (child)
                child->print(depth + 1);
    }

    string getValue(const string& _type) {
        for (auto* child : this->children)
            if (child->children.empty() && child->type == _type)
                return child->value;
        return "";
    }

    Node* getNode(const string& _type) {
        for (auto* child : this->children)
            if (!child->children.empty() && child->type == _type && child->value.empty())
                return child;
        return nullptr;
    }
 
    ~Node() {
        for (auto child : children)
            delete child;
    }

};



enum ScopeError {
    none,
    UndeclaredVariable,
    UndefinedFunction,
    VariableRedefinition,
    FunctionRedefinition
};



enum Context {
    variable,
    function,
    argument,
    parameter,
    error
};



struct Symbol {
    string name;
    string type;
    Context context;
    ScopeError error;
};



struct Scope {

    string name;
    Scope* parent = nullptr;
    vector<Symbol> symbols;
    vector<Scope*> children;


    void print() {
        printScope(this, 0);
    }

    bool isEmpty(Scope* _scope) {
        if (!_scope)
            return true;

        if (!_scope->symbols.empty())
            return false;

        for (auto child : _scope->children)
            if (!isEmpty(child))
                return false;

        return true;
    }

    void printScope(Scope* _scope, int indent) {

        if (!_scope) return;

        if (isEmpty(_scope)) return;

        string spacing(indent * 2, ' ');
        cout << spacing << "[\033[0;90m" << _scope->name << "\033[0m]\n";

        for (const auto& symbol : _scope->symbols) {
            if (symbol.context == Context::error) {
                cout << spacing << "  \033[1;31mWarning\033[0m: " << symbol.name << endl;
            }
            else {
                cout << spacing << "  " << symbol.name << " : \033[0;36m";
                switch (symbol.context) {
                case Context::variable: cout << "variable "; break;
                case Context::function: cout << "function "; break;
                case Context::argument: cout << "argument "; break;
                case Context::parameter: cout << "parameter"; break;
                }
                cout << "\033[0m" << " (\033[0;91m" << symbol.type << "\033[0m)\n";
            }
        }
        cout << endl;

        for (auto child : _scope->children)
            printScope(child, indent + 1);

    }

};