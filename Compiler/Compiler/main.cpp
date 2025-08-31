#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
using namespace std;

struct token {
    string type;
    string value;
};

vector<token> tokenize(const string& code) {
    vector<token> tokens;

    regex whitespace("^\\s+");
    regex keyword("^(int|float|double|char|string|bool|void|return|if|else|while|for|do|cout|cin)");
    regex identifier("^[_A-Za-z][_A-Za-z0-9]*");
    regex number("^[0-9]+");
    regex arithmetic_op("^(\\+\\+|\\-\\-|\\+|\\-|\\*|/|%)");
    regex comparison_op("^(==|!=|<=|>=|>|<)");
    regex assignment_op("^(\\+=|\\-=|\\*=|/=|%=|=)");
    regex separator("^(;|,|\\(|\\)|\\{|\\})");
    regex strLiteral("^\"([^\"\\\\]|\\\\.)*\"");

    int i = 0;
    while (i < code.size()) {
        string snippet = code.substr(i);
        smatch m;

        if (regex_search(snippet, m, whitespace)) {
            i += m.length();
        }
        else if (regex_search(snippet, m, keyword)) {
            tokens.push_back({ "KEYWORD", m.str()});
            i += m.length();
        }
        else if (regex_search(snippet, m, identifier)) {
            tokens.push_back({ "IDENTIFIER", m.str() });
            i += m.length();
        }
        else if (regex_search(snippet, m, assignment_op)) {
            tokens.push_back({ "ASSIGNMENT", m.str() });
            i += m.length();
        }
        else if (regex_search(snippet, m, number)) {
            tokens.push_back({ "NUMBER", m.str() });
            i += m.length();
        }
        else if (regex_search(snippet, m, strLiteral)) {
            tokens.push_back({ "STRING", m.str() });
            i += m.length();
        }
        else if (regex_search(snippet, m, separator)) {
            tokens.push_back({ "SEPERATOR", m.str() });
            i += m.length();
        }
        else {
            cout << "UNKNOWN" << endl;
            i++;
        }
    }
    return tokens;
}

int main() {
    ifstream file("code.txt");
    string code((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    vector<token> tokens = tokenize(code);

    for (auto &i : tokens) {
        cout << '<' << i.type << ": " << i.value << '>' << endl;
    }
}
