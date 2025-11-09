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
            cout << "\033[0;31m" << value << "\033[0m" << endl;
            exit(1);
        }

        cout << type;
        if (!value.empty())
            cout << ": " << value;
        cout << endl;

        for (Node* child : children)
            if (child)
                child->print(depth + 1);
    }

    ~Node() {
        for (auto* c : children)
            delete c;
    }

};
