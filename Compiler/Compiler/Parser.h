#pragma once
#include <iostream>
#include <vector>
#include <stdexcept>
#include <set>
#include "Tokenizer.h"
using namespace std;



runtime_error UnexpectedEOF(const string& expected) {
	if (expected == "value" || expected == "type" || expected == "declaration") return runtime_error("UnexpectedEOF: expected a " + expected);
	else return runtime_error("UnexpectedEOF: expected \"" + expected + "\"");
}

runtime_error UnexpectedToken(const string& expected, const token& encountered) {
	string value;
	if (expected == "value" || expected == "type" || expected == "declaration") value = expected;
	else value = "\"" + expected + "\"";
	if (encountered.type == "UNDEFINED") {
		string undefined = encountered.value == "\"" ? "\\\"" : encountered.value;
		return runtime_error("UnexpectedToken: expected a " + value + ", got \"" + undefined + "\"");
	}
	return runtime_error("UnexpectedToken: expected a " + value + ", got \"" + encountered.type + "\"");
}

runtime_error ExpectedIdentifier(const string& encountered) {
	return runtime_error("ExpectedIdentifier: \"" + encountered + "\" identified");
}

runtime_error UndefinedToken(const string& encountered) {
	if (encountered == "\"") return runtime_error("UndefinedToken: \"\\" + encountered + "\" found");
	else return runtime_error("UndefinedToken: \"" + encountered + "\" found");
}

runtime_error ExpectedTypeToken() {
	return runtime_error("ExpectedTypeToken: unable to find data type");
}

runtime_error ExpectedIntLit(const string& encountered) {
	return runtime_error("ExpectedIntLit: expected an integer, got " + encountered);
}

runtime_error ExpectedFloatLit(const string& encountered) {
	return runtime_error("ExpectedFloatLit: expected a float value, got " + encountered);
}

runtime_error ExpectedStringLit(const string& encountered) {
	return runtime_error("ExpectedStringLit: expected a string literal, got " + encountered);
}

runtime_error ExpectedCharacterLit(const string& encountered) {
	return runtime_error("ExpectedCharacterLit: expected a character, got " + encountered);
}

runtime_error ExpectedBooleanValue(const string& encountered) {
	return runtime_error("ExpectedBooleanValue: expected a Boolean expression, got " + encountered);
}

runtime_error ExpectedExpression(const string& expected) {
	return runtime_error("ExpectedExpression: expected an assignment (" + expected + ")");
}



class Parser
{
private:

	vector<token> tokens;
	size_t cursor;
	int indent = 0;

	set<string> types = { "int", "float", "double", "char", "string", "bool" };
	set<string> literals = { "INTEGER", "DECIMAL", "STRLITERAL", "CHARACTER", "true", "false" };


	void printToken(bool comma = true) {
		for (int i = 0; i < indent; i++) cout << "   ";
		cout << tokens[cursor - 1].type << ": "
			<< ((tokens[cursor - 1].type == "STRLITERAL" || tokens[cursor - 1].type == "HEADER" || tokens[cursor - 1].type == "CHARACTER") ? "" : "\"")
			<< tokens[cursor - 1].value
			<< ((tokens[cursor - 1].type == "STRLITERAL" || tokens[cursor - 1].type == "HEADER" || tokens[cursor - 1].type == "CHARACTER") ? "" : "\"")
			<< (comma ? "," : "") << endl;
	}

	void printRule(const string& message) const {
		for (int i = 0; i < indent; i++) cout << "   ";
		cout << message << endl;
	}

	token& currentToken(const string& expected = "") {
		if (cursor >= tokens.size())
			throw UnexpectedEOF(expected);
		return tokens[cursor];
	}

	void expect(const string& type, const string& value = "") {
		if (cursor >= tokens.size())
			throw UnexpectedEOF(type);
		if (tokens[cursor].type == "UNDEFINED") throw UndefinedToken(tokens[cursor].value);
		else if (currentToken().type != type) {
			if (type == "IDENTIFIER") throw ExpectedIdentifier(currentToken().type);
			throw UnexpectedToken(type, currentToken());
		}
		else if (!value.empty() && currentToken().value != value) { // type is expected, but not the value
			throw UnexpectedToken(value, currentToken());
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
			if (currentToken("LIBRARY").type == "LIBRARY" || currentToken("HEADER").type == "HEADER") {
				expect(currentToken().type);
				printToken(false);
			}
			else if (tokens[cursor].type == "UNDEFINED") expect("UNDEFINED");
			else throw UnexpectedToken("LIBRARY\" or \"HEADER", tokens[cursor]);
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

	void parse_declarations() {
		indent++;
		printRule("declarations {");
		parse_declaration();
		printRule("}");
		indent--;
	}

	void parse_declaration() {
		indent++;
		printRule("declaration {");
		while (cursor < tokens.size()) {
			if (currentToken("declaration").value == "#define")	parse_define();
			//else if (currentToken("class").value == "class" || currentToken("struct").value == "struct") parse_object();
			else parse_declare();
		}
		printRule("}");
		indent--;
	}

	void parse_define() {
		indent++;
		printRule("define {");
		indent++;
		expect("KEYWORD", "#define");
		printToken(true);
		expect("IDENTIFIER");
		printToken(true);
		if (literals.count(currentToken("value").type) || literals.count(currentToken().value)) {
			expect(currentToken().type);
			printToken(false);
			indent--;
		}
		else throw UnexpectedToken("value", currentToken());
		printRule("}");
		indent--;
	}

	void parse_declare();
	void parse_expression(const string& type);
	void parse_term(const string& type);
	void parse_subterm(const string& type);
	void parse_factor(const string& type);
	void parse_subfactor(const string& type);

	void parse_functionCall();
	void parse_parameters();
	void parse_parameter();

	//void parse_object();
	//void parse_objBody();
	//void parse_objBlock();

	void parse_function(const string& type);
	void parse_voidfunction();
	void parse_mainfunction();
	void parse_arguments();
	void parse_argument();

	void parse_statements();
	void parse_statement();
	void parse_iostream(const string& stream);
	void parse_ostring();

	void parse_return(const string& type);


public:

	Parser(const vector<token>& stream) : tokens(stream), cursor(0) {}

	void parse() {
		printRule("program {");
		try {
			parse_headers();
			parse_declarations();
			printRule("}");
		}
		catch (const runtime_error& e) {
			printRule(e.what());
		}
	}
};

#include "ParseDeclare.h"
#include "ParseFunction.h"
#include "ParseStatement.h"