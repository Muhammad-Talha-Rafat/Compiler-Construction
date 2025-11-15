#pragma once
#include <iostream>
#include <vector>
#include <stdexcept>
#include <set>
#include "Tokenizer.h"
using namespace std;


namespace ParseError {

	runtime_error UnexpectedEOF() {
		return runtime_error("\033[0;31mUnexpectedEOF\033[0m: end of code was not expected");
	}

	runtime_error UnexpectedToken(const string& expected, const token& encountered) {
		string value;
		if (expected == "value" || expected == "type" || expected == "declaration") value = expected;
		else value = "\"" + expected + "\"";
		if (encountered.type == "UNDEFINED") {
			string undefined = encountered.value == "\"" ? "\\\"" : encountered.value;
			return runtime_error("\033[0;31mUnexpectedToken\033[0m: expected a " + value + ", got \"" + undefined + "\"");
		}
		return runtime_error("\033[0;31mUnexpectedToken\033[0m: expected " + value + ", got \"" + encountered.type + "\"");
	}

	runtime_error ExpectedIdentifier(const string& encountered) {
		return runtime_error("\033[0;31mExpectedIdentifier\033[0m: expected an <IDENTIFIER> but \"" + encountered + "\" identified");
	}

	runtime_error UndefinedToken(const string& encountered) {
		if (encountered == "\"") return runtime_error("\033[0;31mUndefinedToken\033[0m: \"\\" + encountered + "\" found");
		else return runtime_error("\033[0;31mUndefinedToken\033[0m: \"" + encountered + "\" found");
	}

	runtime_error ExpectedTypeToken() {
		return runtime_error("\033[0;31mExpectedTypeToken\033[0m: unable to find data type");
	}

	runtime_error ExpectedIntLit(const string& type) {
		return runtime_error("\033[0;31mExpectedIntLit\033[0m: expected an integer value for type <" + type + ">");
	}

	runtime_error ExpectedFloatLit(const string& type) {
		return runtime_error("\033[0;31mExpectedFloatLit\033[0m: expected a float value for type <" + type + ">");
	}

	runtime_error ExpectedStringLit(const string& type) {
		return runtime_error("\033[0;31mExpectedStringLit\033[0m: expected a string literal for type <" + type + ">");
	}

	runtime_error ExpectedCharacterLit(const string& type) {
		return runtime_error("\033[0;31mExpectedCharacterLit\033[0m: expected a character for type <" + type + ">");
	}

	runtime_error ExpectedBooleanValue(const string& type) {
		return runtime_error("\033[0;31mExpectedBooleanValue\033[0m: expected a bool value for type <" + type + ">");
	}

	runtime_error ExpectedExpression() {
		return runtime_error("\033[0;31mExpectedExpression\033[0m: an expression was expected here");
	}

	runtime_error SyntaxError(const string& message) {
		return runtime_error("\033[1;31mSyntax error\033[0m: " + message);
	}

}


bool hasParseError(Node* root) {
	Node* temp = root;
	while (temp && !temp->children.empty()) {
		temp = temp->children.back();
	}
	if (temp && temp->type == "error") {
		cout << "[\033[1;31mCritical\033[0m] Unfortunately an error has been encountered during parsing"
			<< "\ncannot do scope analysis, print the AST to further information\n";
		return true;
	}
	else return false;
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
		else throw ParseError::UnexpectedEOF();
	}

	token consume() {
		if (cursor < tokens.size())
			return tokens[cursor++];
		else throw ParseError::UnexpectedEOF();
	}

	token expectType(const string& expected) {
		if (peek().type == expected)
			return consume();
		else throw ParseError::UnexpectedToken(expected, peek());
	}

	token expectValue(const string& expected) {
		if (peek().value == expected)
			return consume();
		else throw ParseError::UnexpectedToken(expected, peek());
	}

	void ignore() {
		if (cursor < tokens.size())
			cursor++;
		else throw ParseError::UnexpectedEOF();
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
			throw ParseError::SyntaxError("program is missing the \"main\" function");
	}
	catch (const runtime_error& e) {
		root->children.push_back(new Node("error", e.what()));
	}

	return root;
}