#pragma once
#include <vector>
#include <regex>
#include "Tokenizer.h"
using namespace std;

class Parser
{
private:

	vector<token> Tokens;
	int cursor = 0;
	vector<token> Queue;

public:

	Parser(vector<token> tokens) :Tokens(tokens) {}

	bool isHeaders() {
		if (!isHeader()) return false;
		else isHeaders();
		return true;
	}

	bool isHeader() {
		if (Tokens[cursor].value == "#include") {
			Queue.push_back(Tokens[cursor]);
			cout << Tokens[cursor];
			cursor++;
			if (Tokens[cursor].type == "LIBRARY" || Tokens[cursor].type == "HEADER") {
				Queue.push_back(Tokens[cursor]);
				cout << Tokens[cursor];
				cursor++;
				return true;
			}
			else cerr << "Syntax error at token" << cursor + 1;
		}
		else if (Tokens[cursor].value == "using") {
			Queue.push_back(Tokens[cursor]);
			cout << Tokens[cursor];
			cursor++;
			if (Tokens[cursor].value == "namespace") {
				Queue.push_back(Tokens[cursor]);
				cout << Tokens[cursor];
				cursor++;
				if (Tokens[cursor].type == "IDENTIFIER") {
					Queue.push_back(Tokens[cursor]);
					cout << Tokens[cursor];
					cursor++;
					if (Tokens[cursor].type == "SEMICOLON") {
						cursor++;
						return true;
					}
					else cerr << "Syntax error at token" << cursor + 1;
				}
				else cerr << "Syntax error at token" << cursor + 1;
			}
			else cerr << "Syntax error at token" << cursor + 1;
		}
		return false;
	}


};

// program		-> headers (declaration | block)
// headers		-> header headers | e
// header		-> <KEYWORD: #include> (<LIBRARY> | <HEADER>) | <using> <namespace> <IDENTIFIER> <SEMICOLON>
// 
// 
// 
// 
// declaration	-> <int> <IDENTIFIER> (<ASSIGN> <INTEGER> | <L_PAREN> (declaration) <R_PAREN> <L_BRACE> block <R_BRACE>) (<SEMICOLON> | <COMMA>)
//					| <string> <IDENTIFIER> (<ASSIGN> <STRLITERAL> | <L_PAREN> <R_PAREN> <L_BRACE> block <R_BRACE>) (<SEMICOLON> | <COMMA>)
