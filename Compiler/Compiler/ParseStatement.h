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
		if (assignment_op.count(currentToken().type)) { // assignment
			printRule("assignment {");
			indent++;
			printToken(true);
			expect(currentToken().type);
			printToken(true);
			parse_expression();
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
			throw runtime_error("Syntax error: expected a valid statement");
		}
	}
	else if (currentToken().value == "const" || currentToken().value == "static" || types.count(currentToken().value)) {
		indent--;
		parse_declare(); // declaration / initialization
		indent++;
	}
	else if (currentToken().value == "++" || currentToken().value == "--") { // pre increment/decrement
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
	else if (currentToken().value == "cout" || currentToken().value == "cin") { // I/O
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
	else if (currentToken().value == "if") { // if block
		printRule("if block {");
		indent++;
		expect(currentToken().type);
		printToken(true);
		expect("lBRACE");
		printToken(true);
		if (currentToken().type != "rBRACE") {
			printRule("conditions {");
			indent++;
			parse_condition();
			indent--;
			printRule("}");
		}
		expect("rBRACE");
		printToken(true);
		// {block}
		expect("lPARENTHESIS");
		printToken(true);
		if (currentToken().type != "rPARENTHESIS") parse_statements();
		expect("rPARENTHESIS");
		printToken(false);
		indent--;
		printRule("}");

		while (currentToken().value == "else if") {
			printRule("elseif block {");
			indent++;
			expect(currentToken().type);
			printToken(true);
			expect("lBRACE");
			printToken(true);
			if (currentToken().type != "rBRACE") {
				printRule("conditions {");
				indent++;
				parse_condition();
				indent--;
				printRule("}");
			}
			expect("rBRACE");
			printToken(true);
			// {block}
			expect("lPARENTHESIS");
			printToken(true);
			if (currentToken().type != "rPARENTHESIS") parse_statements();
			expect("rPARENTHESIS");
			printToken(false);
			indent--;
			printRule("}");
		}

		if (currentToken().value == "else") {
			printRule("else block {");
			indent++;
			expect(currentToken().type);
			printToken(true);
			// no condition
			expect("lPARENTHESIS");
			printToken(true);
			if (currentToken().type != "rPARENTHESIS") parse_statements();
			expect("rPARENTHESIS");
			printToken(false);
			indent--;
			printRule("}");

		}
	}
	else if (currentToken().value == "else if" || currentToken().value == "else") {
		expect(currentToken().type);
		printToken(true);
		throw runtime_error("Syntax error: expected an 'if' statement before this");
	}
	else if (currentToken().value == "for") { // 
		printRule("for loop {");
		indent++;
		expect(currentToken().type);
		printToken(true);
		expect("lBRACE");
		printToken(true);
		// assignment
		printRule("init {");
		indent++;
		string type = "int";
		if (types.count(currentToken().value)) {
			type = currentToken().value;
			expect(currentToken().type);
			printToken(true);
		}
		expect("IDENTIFIER");
		printToken(true);
		expect("ASSIGN");
		printToken(true);
		if (currentToken().type == "SEMICOLON")
			throw ExpectedExpression(type);
		parse_expression(type);
		indent--;
		printRule("}");
		expect("SEMICOLON");
		printToken(true);
		// condition
		printRule("conditions {");
		indent++;
		if (currentToken().type == "SEMICOLON")
			throw UnexpectedToken("condition", currentToken());
		parse_condition();
		indent--;
		printRule("}");
		expect("SEMICOLON");
		printToken(true);
		// update
		printRule("update {");
		indent++;
		if (currentToken().type == "INCREMENT" || currentToken().type == "DECREMENT") {
			printRule(currentToken().value == "++" ? "increment {" : "decrement {");
			indent++;
			expect(currentToken().type);
			printToken(true);
			expect("IDENTIFIER");
			printToken(false);
			indent--;
			printRule("}");
		}
		else if (currentToken().type == "IDENTIFIER") {
			expect(currentToken().type);
			if (currentToken().type == "INCREMENT" || currentToken().type == "DECREMENT") {
				printRule(currentToken().value == "++" ? "increment {" : "decrement {");
				indent++;
				printToken(true);
				expect(currentToken().type);
				printToken(false);
				indent--;
			}
		}
		else throw runtime_error("Syntax error: give a valid update statement");
		indent--;
		printRule("}");
		expect("rBRACE");
		printToken(true);
		// {block}
		expect("lPARENTHESIS");
		printToken(true);
		if (currentToken().type != "rPARENTHESIS") parse_statements();
		expect("rPARENTHESIS");
		printToken(true);
		indent--;
		printRule("}");
	}
	else if (currentToken().value == "while") {
		printRule("while loop {");
		indent++;
		expect(currentToken().type);
		printToken(true);
		expect("lBRACE");
		printToken(true);
		if (currentToken().type != "rBRACE")
			parse_condition();
		expect("rBRACE");
		printToken(true);
		// {block}
		expect("lPARENTHESIS");
		printToken(true);
		if (currentToken().type != "rPARENTHESIS")
			parse_statements();
		expect("rPARENTHESIS");
		printToken(true);
		indent--;
		printRule("}");
	}
	else if (currentToken().value == "do") {
		printRule("do while loop {");
		indent++;
		expect(currentToken().type);
		printToken(true);
		// {block}
		expect("lPARENTHESIS");
		printToken(true);
		if (currentToken().type != "rPARENTHESIS")
			parse_statements();
		expect("rPARENTHESIS");
		printToken(true);
		// (conditions)
		expect("KEYWORD", "while");
		printToken(true);
		expect("lBRACE");
		printToken(true);
		if (currentToken().type != "rBRACE")
			parse_condition();
		expect("rBRACE");
		printToken(true);
		expect("SEMICOLON");
		printToken(true);
		indent--;
		printRule("}");
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
	if (currentToken() == token{ "KEYWORD", "endl" }) {
		expect(currentToken().type);
		printToken(true);
	}
	else parse_expression();
	if (currentToken().type == "LEFT_SHIFT") parse_ostring();
}

void Parser::parse_condition() {
	printRule("condition {");
	indent++;
	parse_expression();
	if (!comparison_op.count(currentToken().type))
		throw UnexpectedToken("COMPARISON op", currentToken());
	expect(currentToken().type);
	printToken(true);
	parse_expression();
	indent--;
	printRule("}");
	if (logical_op.count(currentToken().type)) {
		expect(currentToken().type);
		printToken(true);
		if (currentToken().type == "lBRACE") {
			expect(currentToken().type);
			printToken(true);
			printRule("conditions {");
			indent++;
			parse_condition();
			indent--;
			printRule("}");
			expect("rBRACE");
			printToken(false);
		}
		else parse_condition();
	}
}
