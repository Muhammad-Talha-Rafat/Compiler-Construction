#pragma once
#include <fstream>
#include <string>
#include <set>
#include <vector>
using namespace std;

set<string> KEYWORDS = { "#include", "#define", "using", "namespace",
                        "int", "float", "double", "char", "string", "bool", "void",
                        "main", "return", "endl", "cout", "cin", "auto",
                        "do", "while", "for", "break", "continue", "true", "false",
                        "if", "else if", "else", "switch", "case", "default", "try", "catch", "throw",
                        "static", "const", "new", "delete", "sizeof", "this",
                        "class", "struct", "template", "enum", "public", "private", "protected" };


set<string> USERDEFINED = { "LIBRARY", "IDENTIFIER", "DECIMAL", "INTEGER", "CHARACTER", "STRLITERAL" };

struct token {
    string type;
    string value;
};

void runWithoutRegex(const string& filename) {
	ifstream file(filename);

	string code((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
	file.close();

	if (!code.empty() && code[0] == '\xEF') code.erase(0, 3);

	cout << code;
}