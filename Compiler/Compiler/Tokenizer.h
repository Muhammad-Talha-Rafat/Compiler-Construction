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
};


struct token {
    string type;
    string value;

    void print() const {
        set<string> USERDEFINED = { "LIBRARY", "HEADER", "IDENTIFIER", "DECIMAL", "INTEGER", "CHARACTER", "STRLITERAL", "UNDEFINED"};
        if (USERDEFINED.find(this->type) != USERDEFINED.end())
            cout << '<' << this->type << ": " << this->value << '>';
        else if (type == "KEYWORD") cout << '<' << this->value << '>';
        else cout << '<' << this->type << '>';
        if (value == "{" || value == "}" || value == ";" || type == "LIBRARY" || type == "HEADER") cout << endl;
    }

    friend ostream& operator<<(ostream& out, const token& show) {
        out << '<' << show.type << ": " << show.value << '>';
        return out;
    }
};

set<string> KEYWORDS = { "#include", "#define", "using", "namespace",
                        "int", "float", "double", "char", "string", "bool", "void",
                        "main", "return", "endl", "cout", "cin", "auto",
                        "do", "while", "for", "break", "continue", "true", "false",
                        "if", "else if", "else", "switch", "case", "default", "try", "catch", "throw",
                        "static", "const", "new", "delete", "sizeof", "this",
                        "class", "struct", "template", "enum", "public", "private", "protected" };

vector<rule> Rules = {
    {"WHITESPACE", regex("^\\s+")},
    {"COMMENT", regex("^(//.*|/\\*[^*]*\\*+([^/*][^*]*\\*+)*/)")},

    {"LIBRARY", regex("^<[^>]+>")},
    {"HEADER", regex("^\"[^\"]+\\.h\"")},

    {"IDENTIFIER", regex("^#?[_A-Za-z][_A-Za-z0-9]*")},

    {"DECIMAL", regex("^[0-9]+\\.[0-9]+")},
    {"INTEGER", regex("^[0-9]+")},
    {"CHARACTER", regex("^'(\\.|[^'\\\\])'")},
    {"STRLITERAL", regex("^\"([^\"\\\\]|\\\\.)*\"")},

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
    {"MEMBEROF", regex("^->")},

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

    {"lBRACE", regex("^\\(")},
    {"rBRACE", regex("^\\)")},
    {"lPARANTHESIS", regex("^\\{")},
    {"rPARANTHESIS", regex("^\\}")},
    {"lBRACKET", regex("^\\[")},
    {"rBRACKET", regex("^\\]")}
};

static vector<token> tokenize(const string& filename) {
    ifstream file(filename);
    string code((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    if (!code.empty() && code[0] == '\xEF') code.erase(0, 3);

    vector<token> tokens;
    size_t cursor = 0;

    while (cursor < code.size()) {
        string snippet = code.substr(cursor);
        smatch match;
        bool matched = false;

        for (auto& rule : Rules) {
            if (regex_search(snippet, match, rule.Regex) && match.position() == 0) {
                if (rule.type != "WHITESPACE" && rule.type != "COMMENT") {
                    if (!tokens.empty() && tokens.back().value != "#include" && rule.type == "HEADER" )
                        tokens.push_back({ "STRLITERAL", match.str() });
                    else if (KEYWORDS.find(match.str()) != KEYWORDS.end())
                        tokens.push_back({ "KEYWORD", match.str() });
                    else tokens.push_back({ rule.type, match.str() });
                }
                cursor += match.length();
                matched = true;
                break;
            }
        }

        if (!matched) {
            tokens.push_back({ "UNDEFINED", string(1, code[cursor]) });
            cursor++;
        }
    }
    return tokens;
}