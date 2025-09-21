#pragma once
#include <iostream>
#include <vector>
#include <stdexcept>
#include "Tokenizer.h"
using namespace std;



runtime_error UnexpectedEOF(const string& expected) {
	return runtime_error("UnexpectedEOF: expected \"" + expected + "\"");
}

runtime_error UnexpectedToken(const string& expected, const string& encountered) {
	return runtime_error("UnexpectedToken: expected \"" + expected + "\", got \"" + encountered + "\"");
}

runtime_error ExpectedIdentifier(const string& encountered) {
	return runtime_error("ExpectedIdentifier: \"" + encountered + "\" identified");
}


class Parser
{
private:

	vector<token> tokens;
	size_t cursor;
	int indent = 0;

	void printToken(bool comma = true) {
		for (int i = 0; i < indent; i++) cout << "   ";
		cout << tokens[cursor - 1].type << ": "
			<< ((tokens[cursor - 1].type == "STRLITERAL" || tokens[cursor - 1].type == "HEADER") ? "" : "\"")
			<< tokens[cursor - 1].value
			<< ((tokens[cursor - 1].type == "STRLITERAL" || tokens[cursor - 1].type == "HEADER") ? "" : "\"")
			<< (comma ? "," : "") << endl;
	}

	void printRule(const string& message) const {
		for (int i = 0; i < indent; i++) cout << "   ";
		cout << message << endl;
	}

	token& currentToken() {
		if (cursor >= tokens.size())
			throw runtime_error("currentToken UnexpectedEOF");
		return tokens[cursor];
	}

	void expect(const string& type, const string& value = "") {
		if (cursor >= tokens.size())
			throw UnexpectedEOF(type);
		if (currentToken().type != type) {
			if (type == "IDENTIFIER") throw ExpectedIdentifier(currentToken().type);
			throw UnexpectedToken(type, currentToken().type);
		}
		if (!value.empty() && currentToken().value != value) { // type is expected, but not the value
			throw UnexpectedToken(value, currentToken().value);
		}
		cursor++;
	}

	void parse_headers() {
		indent++;
		printRule("headers {");
		while (cursor < tokens.size() && (currentToken().value == "#include" || currentToken().value == "using"))
			parse_header();
		printRule("}");
		indent--;
	}

	void parse_header() {
		indent++;
		printRule("header {");
		if (currentToken().value == "#include") {
			indent++;
			expect("KEYWORD", "#include");
			printToken(true);
			if (currentToken().type == "LIBRARY" || currentToken().type == "HEADER") {
				expect(currentToken().type);
				printToken(false);
			}
			else throw UnexpectedToken("LIBRARY\" or \"HEADER", tokens[cursor].type);
			indent--;
		}
		else if (currentToken().value == "using") {
			indent++;
			expect("KEYWORD", "using");
			printToken(true);
			expect("KEYWORD", "namespace");
			printToken(true);
			expect("IDENTIFIER");
			printToken(true);
			expect("SEMICOLON");
			printToken(false);
			indent--;
		}
		printRule("}");
		indent--;
	}

public:

	Parser(const vector<token>& stream) : tokens(stream), cursor(0) {}

	void parse() {
		printRule("program {");
		try {
			parse_headers();
			printRule("}");
		}
		catch (const runtime_error& e) {
			printRule(e.what());
		}
	}
};