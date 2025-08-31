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


regex whitespace("^\\s+");
regex comment("^(//.*|/\\*[^*]*\\*+([^/*][^*]*\\*+)*/)");
regex keyword("^(int|float|double|char|string|bool|void|return|if|else if|else|while|for|do|cout|cin|struct|class|public|private|protected|static|const|using|namespace|endl)");
regex library("^#include\\s*(<[^>]+>|\"[^\"]+\")");
regex identifier("^[_A-Za-z][_A-Za-z0-9]*");
regex number("^[0-9]+");
regex arithmetic_op("^(\\+\\+|\\-\\-|\\+|\\-|\\*|/|%)");
regex comparison_op("^(==|!=|<=|>=|>|<)");
regex assignment_op("^(\\+=|\\-=|\\*=|/=|%=|<<=|>>=|=)");
regex logical_op("^(&&|\\|\\||!)");
regex bitwise_op("^(<<|>>|\\^|~|&|\\|)");
regex lBrace("^\\(");
regex rBrace("^\\)");
regex lParenthesis("^\\{");
regex rParenthesis("^\\}");
regex lBracket("^\\[");
regex rBracket("^\\]");
regex separator("^(;|,)");
regex strLiteral("^\"([^\"\\\\]|\\\\.)*\"");


static vector<token> tokenize(const string& code) {
    vector<token> tokens;
    int i = 0;

    while (i < code.size()) {
        string snippet = code.substr(i);
        smatch substring;

        if (regex_search(snippet, substring, whitespace) && substring.position() == 0) {
            i += substring.length();
        }
        else if (regex_search(snippet, substring, comment) && substring.position() == 0) {
            tokens.push_back({ "COMMENT", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, library) && substring.position() == 0) {
            tokens.push_back({ "LIBRARY", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, keyword) && substring.position() == 0) {
            tokens.push_back({ "KEYWORD", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, identifier) && substring.position() == 0) {
            tokens.push_back({ "IDENTIFIER", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, logical_op) && substring.position() == 0) {
            tokens.push_back({ "LOGIC", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, bitwise_op) && substring.position() == 0) {
            tokens.push_back({ "BITWISE", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, assignment_op) && substring.position() == 0) {
            tokens.push_back({ "ASSIGNMENT", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, comparison_op) && substring.position() == 0) {
            tokens.push_back({ "COMPARATOR", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, arithmetic_op) && substring.position() == 0) {
            tokens.push_back({ "OPERATOR", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, number) && substring.position() == 0) {
            tokens.push_back({ "NUMBER", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, strLiteral) && substring.position() == 0) {
            tokens.push_back({ "STRING", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, lBrace) && substring.position() == 0) {
            tokens.push_back({ "L_Brace", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, rBrace) && substring.position() == 0) {
            tokens.push_back({ "R_Brace", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, lParenthesis) && substring.position() == 0) {
            tokens.push_back({ "L_PARENTHESIS", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, rParenthesis) && substring.position() == 0) {
            tokens.push_back({ "R_PARENTHESIS", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, lBracket) && substring.position() == 0) {
            tokens.push_back({ "L_Bracket", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, rBracket) && substring.position() == 0) {
            tokens.push_back({ "R_Bracket", substring.str() });
            i += substring.length();
        }
        else if (regex_search(snippet, substring, separator) && substring.position() == 0) {
            tokens.push_back({ "SEPERATOR", substring.str() });
            i += substring.length();
        }
        else {
            tokens.push_back({ "UNDEFINED", "?" });
            i++;
        }
    }
    return tokens;
}

int main() {
    ifstream file("code.txt");

    string code((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    if (!code.empty() && code[0] == '\xEF') {
        code.erase(0, 3);
    }

    vector<token> tokens = tokenize(code);

    for (auto &i : tokens) {
        cout << '<' << i.type << ": " << i.value << '>' << endl;
    }
    return 0;
}
