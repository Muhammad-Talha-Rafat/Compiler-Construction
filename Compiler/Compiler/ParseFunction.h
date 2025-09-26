#pragma once
using namespace std;


void Parser::parse_function() {
	expect("lBRACE");
	printToken(true);
	if (currentToken().type != "rBRACE") // means, there ARE arguments
		parse_arguments();
	expect("rBRACE");
	printToken(true);
	expect("lPARENTHESIS");
	printToken(true);
	// statemnts
	// return
	expect("rPARENTHESIS");
	printToken(true);
}

void Parser::parse_voidfunction() {
	expect("KEYWORD", "void");
	printToken(true);
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
	// statements
	expect("rPARENTHESIS");
	printToken(true);
}

void Parser::parse_mainfunction() {
	expect(currentToken().type);
	printToken(true);
	expect("lBRACE");
	printToken(true);
	if (currentToken().type != "rBRACE")
		throw runtime_error("Syntax error: 'main' could not contain arguments");
	expect("rBRACE");
	printToken(true);
	expect("lPARENTHESIS");
	printToken(true);
	// statements
	// return | ε
	expect("rPARENTHESIS");
	printToken(true);
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
		if (!types.count(currentToken().value)) throw ExpectedTypeToken();
	}
	if (types.count(currentToken().value)) {
		expect(currentToken().type);
		printToken(true);
		expect("IDENTIFIER");
		printToken(false);
	}
	else throw ExpectedTypeToken();
	indent--;
	printRule("}");
	indent--;
}
