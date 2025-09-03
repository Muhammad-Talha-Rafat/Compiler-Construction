#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <set>
using namespace std;

struct rule {
    string type;
    regex Regex;
    bool reserved = 1;
};

set<string> USERDEFINED = { "LIBRARY", "IDENTIFIER", "DECIMAL", "INTEGER", "CHARACTER", "STRLITERAL" };

struct token {
    string type;
    string value;

    void print() {
        if (USERDEFINED.find(this->type) != USERDEFINED.end())
            cout << '<' << this->type << ": " << this->value << '>';
        else if (type == "KEYWORD") cout << '<' << this->value << '>';
        else cout << '<' << this->type << '>';
        if (value == "{" || value == "}" || value == ";" || type == "LIBRARY") cout << endl;
    }
};

set<string> KEYWORDS = { "#include", "#define", "using", "namespace",
                        "int", "float", "double", "char", "string", "bool", "void",
                        "main", "return", "endl", "cout", "cin",
                        "do", "while", "for", "break", "continue", "true", "false",
                        "if", "else if", "else", "switch", "case", "default", "try", "catch", "throw",
                        "static", "const", "new", "delete", "sizeof", "this",
                        "class", "struct", "template", "enum", "public", "private", "protected" };


vector<rule> Rules = {
    {"WHITESPACE", regex("^\\s+")},
    {"COMMENT", regex("^(//.*|/\\*[^*]*\\*+([^/*][^*]*\\*+)*/)")},

    {"LIBRARY", regex("^(<[^>]+>|\"[^\"]+\\.h\")"), 0},

    {"IDENTIFIER", regex("^#?[_A-Za-z][_A-Za-z0-9]*"), 0},

    {"DECIMAL", regex("^[0-9]+\\.[0-9]+"), 0},
    {"INTEGER", regex("^[0-9]+"), 0},
    {"CHARACTER", regex("^'(\\.|[^'\\\\])'"), 0},
    {"STRLITERAL", regex("^\"([^\"\\\\]|\\\\.)*\""), 0},

    {"INCREMENT", regex("^\\+\\+")},
    {"DECREMENT", regex("^\\-\\-")},

    {"EQUALS", regex("^==")},
    {"NOTEQUAL", regex("^!=")},
    {"GREATERorEQUAL", regex("^>=")},
    {"SMALLERorEQUAL", regex("^<=")},

    {"AND_LOGIC", regex("^&&")},
    {"OR_LOGIC", regex("^\\|\\|")},
    {"NOT_LOGIC", regex("^!")},

    {"LEFT_SHIFT", regex("^<<")},
    {"RIGHT_SHIFT", regex("^>>")},

    {"ADD_ASSIGN", regex("^\\+=")},
    {"SUB_ASSIGN", regex("^\\-=")},
    {"MUL_ASSIGN", regex("^\\*=")},
    {"DIV_ASSIGN", regex("^/=")},
    {"MOD_ASSIGN", regex("^%=")},
    {"AND_ASSIGN", regex("^&=")},
    {"OR_ASSIGN", regex("^\\|=")},
    {"XOR_ASSIGN", regex("^\\^=")},
    {"LEFT_SHIFT_ASSIGN", regex("^<<=")},
    {"RIGHT_SHIFT_ASSIGN", regex("^>>=")},

    {"ASSIGN", regex("^=")},
    {"ADDITION", regex("^\\+")},
    {"SUBTRACTION", regex("^\\-")},
    {"MULTIPLICATION", regex("^\\*")},
    {"DIVISION", regex("^/")},
    {"MODULUS", regex("^%")},

    {"GREATERTHAN", regex("^>")},
    {"SMALLERTHAN", regex("^<")},

    {"AND", regex("^&")},
    {"OR", regex("^\\|")},
    {"XOR", regex("^\\^")},
    {"NOT", regex("^~")},

    {"QUESTION", regex("^\\?")},
    {"SCOPE", regex("^::")},
    {"COLON", regex("^:")},
    {"SEMICOLON", regex("^;")},
    {"COMMA", regex("^,")},
    {"DOT", regex("^\\.")},
    {"MEMBEROF", regex("^->")},

    {"lBRACE", regex("^\\(")},
    {"rBRACE", regex("^\\)")},
    {"lPARANTHESIS", regex("^\\{")},
    {"rPARANTHESIS", regex("^\\}")},
    {"lBRACKET", regex("^\\[")},
    {"rBRACKET", regex("^\\]")}
};

static vector<token> tokenize(const string& code) {
    vector<token> tokens;
    size_t i = 0;

    while (i < code.size()) {
        string snippet = code.substr(i);
        smatch match;
        bool matched = false;

        for (auto& rule : Rules) {
            if (regex_search(snippet, match, rule.Regex) && match.position() == 0) {
                if (rule.type != "WHITESPACE" && rule.type != "COMMENT") {
                    if (KEYWORDS.find(match.str()) != KEYWORDS.end())
                        tokens.push_back({ "KEYWORD", match.str() });
                    else tokens.push_back({ rule.type, match.str() });
                }
                i += match.length();
                matched = true;
                break;
            }
        }

        if (!matched) {
            tokens.push_back({ "UNDEFINED", string(1, code[i]) });
            i++;
        }
    }
    return tokens;
}

static void runWithRegex(const string& filename) {
    ifstream file(filename);

    string code((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    file.close();

    if (!code.empty() && code[0] == '\xEF') code.erase(0, 3);

    vector<token> tokens = tokenize(code);

    for (auto& i : tokens) i.print();
}
