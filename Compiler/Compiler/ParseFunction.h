#pragma once
using namespace std;


void Parser::parse_function(const string& type) {
	expect("lBRACE");
	printToken(true);
	if (currentToken().type != "rBRACE") // means, there ARE arguments
		parse_arguments();
	expect("rBRACE");
	printToken(true);
	expect("lPARENTHESIS");
	printToken(true);
	if (currentToken().type != "rPARENTHESIS")
		parse_statements();
	if (currentToken().value != "return")
		throw runtime_error("Runtime error: expected a 'return' statement");
	parse_return(type);
	expect("rPARENTHESIS");
	printToken(true);
}

void Parser::parse_voidfunction() {
	expect("KEYWORD", "void");
	printToken(true);
	if (tokens[cursor].value == "main") {
		expect(currentToken().type);
		printToken(true);
		throw runtime_error("Runtime error: 'main' can only have return type 'int'");
	}
	expect("IDENTIFIER");
	printToken(true);
	expect("lBRACE");
	printToken(true);
	if (currentToken().type != "rBRACE")
		parse_arguments();
	expect("rBRACE");
	printToken(true);
	expect("lPARENTHESIS");
	printToken(true);
	if (currentToken().type != "rPARENTHESIS")
		parse_statements();
	if (currentToken().value == "return") {
		printRule("return {");
		indent++;
		expect(currentToken().type);
		printToken(true);
		if (currentToken().type != "SEMICOLON")
			throw runtime_error("Runtime error: 'void' functions do not return anything");
		expect("SEMICOLON");
		printToken(false);
		indent--;
		printRule("}");
	}
	expect("rPARENTHESIS");
	printToken(false);
}

void Parser::parse_mainfunction() {
	expect(currentToken().type, "main");
	printToken(true);
	if (tokens[cursor - 2].value != "int")
		throw runtime_error("Runtime error: 'main' can only have return type 'int'");
	expect("lBRACE");
	printToken(true);
	if (currentToken().type != "rBRACE")
		throw runtime_error("Syntax error: 'main' could not contain arguments");
	expect("rBRACE");
	printToken(true);
	expect("lPARENTHESIS");
	printToken(true);
	parse_statements();
	if (currentToken() == token{ "KEYWORD", "return" })
		parse_return("int");
	expect("rPARENTHESIS");
	printToken(false);
	indent--;
}

void Parser::parse_arguments() {
	printRule("arguments {");
	parse_argument();
	while (currentToken().type == "COMMA") {
		indent++;
		expect(currentToken().type);
		printToken(true);
		indent--;
		parse_argument();
	}
	printRule("}");
}

void Parser::parse_argument() {
	indent++;
	printRule("argument {");
	indent++;
	if (currentToken().value == "const" || currentToken().value == "static") {
		expect(currentToken().type);
		printToken(true);
	}
	if (!types.count(currentToken().value)) throw ExpectedTypeToken();
	expect(currentToken().type);
	printToken(true);
	expect("IDENTIFIER");
	printToken(false);
	indent--;
	printRule("}");
	indent--;
}

void Parser::parse_return(const string& type) {
	printRule("return {");
	indent++;
	expect("KEYWORD", "return");
	printToken(true);
	if (currentToken().type != "SEMICOLON")
		parse_expression(type);
	expect("SEMICOLON");
	printToken(false);
	indent--;
	printRule("}");
}