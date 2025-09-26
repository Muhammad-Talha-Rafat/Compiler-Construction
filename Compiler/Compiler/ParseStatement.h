#pragma once
using namespace std;


void Parser::parse_statements() {
	set<string> statementset = { "++", "--", "for", "while", "do", "else if", "if", "else", "cout", "cin", "const", "static" };
	printRule("statements {");
	indent++;
	while (currentToken().type == "IDENTIFIER" || statementset.count(currentToken().value) || types.count(currentToken().value))
		parse_statement();
	indent--;
	printRule("}");
}

void Parser::parse_statement() {
	printRule("statement {");
	indent++;
	if (currentToken().type == "IDENTIFIER") {
		expect(currentToken().type);
		set<string> assignment_op = { "+=", "-=", "*=", "/=", "%=", "=" };
		if (assignment_op.count(currentToken().value)) { // assignment
			printRule("assignment {");
			indent++;
			printToken(true);
			expect(currentToken().type);
			printToken(true);
			parse_expression("");
			expect("SEMICOLON");
			printToken(false);
			indent--;
			printRule("}");
		}
		else if (currentToken().type == "lBRACE") { // call statement
			printRule("callStatement {");
			indent++;
			printToken(true);
			expect(currentToken().type);
			printToken(true);
			if (currentToken().type == "IDENTIFIER" || literals.count(currentToken().type))
				parse_parameters();
			else if (tokens[cursor].value == "true" || tokens[cursor].value == "false")
				parse_parameters();
			expect("rBRACE");
			printToken(true);
			expect("SEMICOLON");
			printToken(false);
			indent--;
			printRule("}");
		}
		else if (currentToken().type == "INCREMENT") { // post increment
			printRule("increment {");
			indent++;
			printToken(true);
			expect(currentToken().type);
			printToken(true);
			expect("SEMICOLON");
			printToken(false);
			indent--;
			printRule("}");
		}
		else if (currentToken().type == "DECREMENT") { // post decrement
			printRule("decrement {");
			indent++;
			printToken(true);
			expect(currentToken().type);
			printToken(true);
			expect("SEMICOLON");
			printToken(false);
			indent--;
			printRule("}");
		}
		else {
			printToken(true);
			throw runtime_error("Syntax error: expected an operation");
		}
	}
	else if (currentToken().value == "const" || currentToken().value == "static" || types.count(currentToken().value)) {
		indent--;
		parse_declare();
		indent++;
	}
	else if (currentToken().value == "++" || currentToken().value == "--") {
		if (currentToken().value == "++") printRule("increment {");
		else printRule("decrement {");
		indent++;
		expect(currentToken().type);
		printToken(true);
		expect("IDENTIFIER");
		printToken(true);
		expect("SEMICOLON");
		printToken(false);
		indent--;
		printRule("}");
	}
	else if (currentToken().value == "cout" || currentToken().value == "cin") {
		if (currentToken().value == "cout") {
			printRule("output {");
			indent++;
			parse_iostream("cout");
			indent--;
			printRule("}");
		}
		else {
			printRule("input {");
			indent++;
			parse_iostream("cin");
			indent--;
			printRule("}");
		}
	}
	indent--;
	printRule("}");
}

void Parser::parse_iostream(const string& stream) {
	if (stream == "cout") {
		expect("KEYWORD", "cout");
		printToken(true);
		parse_ostring();
		expect("SEMICOLON");
		printToken(false);
	}
	else { // stream == "cin"
		expect("KEYWORD", "cin");
		printToken(true);
		expect("RIGHT_SHIFT");
		printToken(true);
		expect("IDENTIFIER");
		printToken(true);
		expect("SEMICOLON");
		printToken(false);
	}
}

void Parser::parse_ostring() {
	expect("LEFT_SHIFT");
	printToken(true);
	parse_expression("");
	if (currentToken().type == "LEFT_SHIFT") parse_ostring();
}

// loops
// if