#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <regex>
using namespace std;

struct rule {
    string type;
    regex Regex;
    bool reserved = 1;
};

struct token {
    string type;
    string value;

    void print() {
        if (type == "SEMICOLON" || type == "rPARANTHESIS" || type == "lPARANTHESIS") cout << '<' << type << '>' << endl;
        else if (type == "LIBRARY") cout << '<' << type << ": " << this->value << '>' << endl;
        else if (value.empty()) cout << '<' << type << '>';
        else cout << '<' << this->type << ": " << this->value << '>';
    }
};

vector<rule> Rules = {
    {"WHITESPACE", regex("^\\s+")},
    {"COMMENT", regex("^(//.*|/\\*[^*]*\\*+([^/*][^*]*\\*+)*/)")},

    {"INCLUDE", regex("^#include")},
    {"LIBRARY", regex("^(<[^>]+>|\"[^\"]+\\.h\")"), 0},
    {"USING", regex("^using")},
    {"NAMESPACE", regex("^namespace")},
    {"DEFINE", regex("^#define")},

    {"INT", regex("^int")},
    {"FLOAT", regex("^float")},
    {"DOUBLE", regex("^double")},
    {"CHAR", regex("^char")},
    {"STRING", regex("^string")},
    {"BOOL", regex("^bool")},
    {"VOID", regex("^void")},

    {"MAIN", regex("^main")},
    {"RETURN", regex("^return")},

    {"WHILE", regex("^while")},
    {"DO", regex("^do")},
    {"FOR", regex("^for")},
    {"SWITCH", regex("^switch")},
    {"CASE", regex("^case")},
    {"DEFAULT", regex("^default")},
    {"TRY", regex("^try")},
    {"CATCH", regex("^catch")},
    {"ELSEIF", regex("^else if")},
    {"ELSE", regex("^else")},

    {"TEMPLATE", regex("^template")},
    {"STRUCT", regex("^struct")},
    {"CLASS", regex("^class")},
    {"PUBLIC", regex("^public")},
    {"PRIVATE", regex("^private")},
    {"PROTECTED", regex("^protected")},

    {"STATIC", regex("^static")},
    {"CONST", regex("^const")},
    {"COUT", regex("^cout")},
    {"CIN", regex("^cin")},
    {"ENDL", regex("^endl")},
    {"NEW", regex("^new")},
    {"DELETE", regex("^delete")},
    {"SIZEOF", regex("^sizeof")},
    {"THIS", regex("^this")},

    {"IDENTIFIER", regex("^[_A-Za-z][_A-Za-z0-9]*"), 0},

    {"DECIMAL", regex("^[0-9]+\\.[0-9]+"), 0},
    {"INTEGER", regex("^[0-9]+"), 0},
    {"CHARACTER", regex("^'(\\.|[^'\\\\])'"), 0},
    {"STRLITERAL", regex("^\"([^\"\\\\]|\\\\.)*\""), 0},
    {"TRUE", regex("^true")},
    {"FALSE", regex("^false")},

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
                if (rule.type != "WHITESPACE") {
                    if (rule.reserved) tokens.push_back({ rule.type });
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
