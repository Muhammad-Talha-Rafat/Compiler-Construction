#pragma once
#include <iostream>
#include <vector>
#include <stdexcept>
#include <set>
#include "Tokenizer.h"
using namespace std;



runtime_error UnexpectedEOF() {
	return runtime_error("UnexpectedEOF: end of code was not expected");
}

runtime_error UnexpectedToken(const string& expected, const token& encountered) {
	string value;
	if (expected == "value" || expected == "type" || expected == "declaration") value = expected;
	else value = "\"" + expected + "\"";
	if (encountered.type == "UNDEFINED") {
		string undefined = encountered.value == "\"" ? "\\\"" : encountered.value;
		return runtime_error("UnexpectedToken: expected a " + value + ", got \"" + undefined + "\"");
	}
	return runtime_error("UnexpectedToken: expected " + value + ", got \"" + encountered.type + "\"");
}

runtime_error ExpectedIdentifier(const string& encountered) {
	return runtime_error("ExpectedIdentifier: expected an <IDENTIFIER> but \"" + encountered + "\" identified");
}

runtime_error UndefinedToken(const string& encountered) {
	if (encountered == "\"") return runtime_error("UndefinedToken: \"\\" + encountered + "\" found");
	else return runtime_error("UndefinedToken: \"" + encountered + "\" found");
}

runtime_error ExpectedTypeToken() {
	return runtime_error("ExpectedTypeToken: unable to find data type");
}

runtime_error ExpectedIntLit(const string& type) {
	return runtime_error("ExpectedIntLit: expected an integer value for type <" + type + ">");
}

runtime_error ExpectedFloatLit(const string& type) {
	return runtime_error("ExpectedFloatLit: expected a float value for type <" + type + ">");
}

runtime_error ExpectedStringLit(const string& type) {
	return runtime_error("ExpectedStringLit: expected a string literal for type <" + type + ">");
}

runtime_error ExpectedCharacterLit(const string& type) {
	return runtime_error("ExpectedCharacterLit: expected a character for type <" + type + ">");
}

runtime_error ExpectedBooleanValue(const string& type) {
	return runtime_error("ExpectedBooleanValue: expected a bool value for type <" + type + ">");
}

runtime_error ExpectedExpression() {
	return runtime_error("ExpectedExpression: an expression was expected here");
}

runtime_error SyntaxError(const string& message) {
	return runtime_error("Syntax error: " + message);
}


class Parser
{
private:

	vector<token> tokens;
	size_t cursor;
	bool main = false;

	token peek(int offset = 0) const {
		if (cursor + offset < tokens.size())
			return tokens[cursor + offset];
		else throw UnexpectedEOF();
	}

	token consume() {
		if (cursor < tokens.size())
			return tokens[cursor++];
		else throw UnexpectedEOF();
	}

	token expectType(const string& expected) {
		if (peek().type == expected)
			return consume();
		else throw UnexpectedToken(expected, peek());
	}

	token expectValue(const string& expected) {
		if (peek().value == expected)
			return consume();
		else throw UnexpectedToken(expected, peek());
	}

	void ignore() {
		if (cursor < tokens.size())
			cursor++;
		else throw UnexpectedEOF();
	}


	set<string> types = { "int", "float", "double", "char", "string", "bool" };
	set<string> literals = { "INTEGER", "DECIMAL", "STRLITERAL", "CHARACTER", "true", "false" };
	set<string> add_sub_op = { "ADDITION", "SUBTRACTION" };
	set<string> mul_div_mod_op = { "MULTIPLICATION", "DIVISION", "MODULUS" };
	set<string> comparison_op = { "GREATERTHAN", "SMALLERTHAN", "EQUALS", "NOTEQUAL", "GREATERorEQUAL", "SMALLERorEQUAL" };
	set<string> logical_op = { "AND_LOGIC", "OR_LOGIC" };
	set<string> assignment_op = { "ADD_ASSIGN", "SUB_ASSIGN", "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN", "ASSIGN" };


	Node* parse_headers();
	Node* parse_header();

	Node* parse_declarations();

	Node* parse_define();

	Node* parse_variable();

	Node* parse_expression();
	Node* parse_term();
	Node* parse_factor();

	Node* parse_functionCall();
	Node* parse_arguments();

	Node* parse_object();
	Node* parse_objBlock();


	Node* parse_function(const string& type, const string& name);
	Node* parse_parameters();

	Node* parse_statements();

	Node* parse_increment();
	Node* parse_decrement();

	Node* parse_conditions();
	Node* parse_comparison();

	Node* parse_return();

public:

	Parser(const vector<token>& stream)
		: tokens(stream), cursor(0) {}

	Node* parse();
};



#include "AST.h"
#include "ParseHeader.h"
#include "ParseDeclaration.h"
//#include "ParseObject.h"
#include "ParseFunction.h"
#include "ParseStatement.h"


Node* Parser::parse() {

	Node* root = new Node("program");

	try {
		Node* headers_node = parse_headers();
		if (headers_node)
			root->children.push_back(headers_node);

		Node* declarations_node = parse_declarations();
		if (declarations_node)
			root->children.push_back(declarations_node);
	}
	catch (const runtime_error& e) {
		root->children.push_back(new Node("error", e.what()));
	}

	return root;
}