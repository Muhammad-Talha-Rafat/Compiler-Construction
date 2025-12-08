#pragma once
#include <iostream>
#include <vector>
#include <stdexcept>
#include <set>
#include "Tokenizer.h"

using namespace std;
using namespace ParseError;


bool hasParseError(Node * root) {
	if (root->type == "error")
		return true;
	for (Node* child : root->children) {
		if (hasParseError(child))
			return true;
	}
	return false;
}





class Parser
{
private:

	vector<token> tokens;
	size_t cursor;
	bool main_trigger = false;

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
	set<string> literals = { "INTEGER", "DECIMAL", "STRLITERAL", "CHARACTER", "BOOLEAN"};
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
	Node* parse_argument();

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



#include "ParseHeader.h"
#include "ParseDeclaration.h"
#include "ParseObject.h"
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

		if (!main_trigger)
			throw SyntaxError("program is missing the \"main\" function");
	}
	catch (const runtime_error& e) {
		root->children.push_back(new Node("error", e.what()));
	}

	return root;
}