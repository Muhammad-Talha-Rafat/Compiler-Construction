#pragma once
using namespace std;


void Parser::parse_declare() {
	string type;
	indent++;
	printRule("declare {");
	indent++;
	if (currentToken().value == "const" || currentToken().value == "static") {
		expect(currentToken().type);
		printToken(true);
	}
	if (types.count(currentToken().value)) {
		type = currentToken().value;
		expect(currentToken().type);
		printToken(true);
	}
	else if (currentToken().value == "void") {
		printRule("void {");
		indent++;
		parse_voidfunction();
		indent--;
		printRule("}");
		indent--;
		printRule("}"); // "declarations" block: complete
		indent--;
		return;
	}
	else throw ExpectedTypeToken();
	if (currentToken() == token{ "KEYWORD", "main" }) {
		this->main = true;
		parse_mainfunction();
		printRule("}"); // "declarations" block: complete
		indent--;
		return;
	}
	expect("IDENTIFIER");
	printToken(true);
	if (currentToken().type == "ASSIGN") {
		printRule("variable {");
		indent++;
		expect(currentToken().type);
		if (currentToken().type == "SEMICOLON") throw ExpectedExpression(type);
		printToken(true);
		parse_expression(type);
		expect("SEMICOLON");
		printToken(false);
		indent--;
		printRule("}");
	}
	else if (currentToken().type == "lBRACE") {
		printRule("function {");
		indent++;
		parse_function(type);
		indent--;
		printRule("}");
	}
	else if (currentToken().type == "SEMICOLON") {
		expect(currentToken().type);
		printToken(false);
	}
	else if (currentToken().type != "ASSIGN" && currentToken().type != "SEMICOLON") throw UnexpectedToken(";", tokens[cursor]);
	indent--;
	printRule("}");
	indent--;
}

void Parser::parse_expression(const string& type = "") {
	printRule("expression {");
	parse_term(type);
	parse_subterm(type);
	printRule("}");
}

void Parser::parse_term(const string& type) {
	indent++;
	printRule("term {");
	parse_factor(type);
	parse_subfactor(type);
	printRule("}");
	indent--;
}

void Parser::parse_subterm(const string& type) {
	if (currentToken().type == "ADDITION" || currentToken().type == "SUBTRACTION") {
		indent++;
		printRule("subterm {");
		indent++;
		expect(currentToken().type);
		printToken(true);
		indent--;
		parse_term(type);
		parse_subterm(type);
		printRule("}");
		indent--;
	}
}

void Parser::parse_factor(const string& type) {
	indent++;
	printRule("factor {");
	indent++;
	if (currentToken().type == "lBRACE") {
		expect(currentToken().type);
		printToken(true);
		parse_expression(type);
		expect("rBRACE");
		printToken(false);
	}
	else if (literals.count(tokens[cursor].type) || tokens[cursor].value == "true" || tokens[cursor].value == "false") {
		if (type == "bool" && tokens[cursor].value != "true" && tokens[cursor].value != "false")
			throw ExpectedBooleanValue(tokens[cursor].type);
		else if (type == "int" && tokens[cursor].type != "INTEGER")
			throw ExpectedIntLit(tokens[cursor].type);
		else if ((type == "float" || type == "double") && tokens[cursor].type != "DECIMAL")
			throw ExpectedFloatLit(tokens[cursor].type);
		else if (type == "char" && tokens[cursor].type != "CHARACTER")
			throw ExpectedCharacterLit(tokens[cursor].type);
		else if (type == "string" && tokens[cursor].type != "STRLITERAL")
			throw ExpectedStringLit(tokens[cursor].type);
		else {
			expect(currentToken().type);
			printToken(false);
		}
	}
	else if (currentToken().type == "INCREMENT") {
		printRule("increment {");
		indent++;
		expect(currentToken().type);
		printToken(true);
		expect("IDENTIFIER");
		printToken(false);
		indent--;
		printRule("}");
	}
	else if (currentToken().type == "DECREMENT") {
		printRule("decrement {");
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
		if (currentToken().type == "INCREMENT") {
			printRule("increment {");
			indent++;
			printToken(true);
			expect(currentToken().type);
			printToken(false);
			indent--;
			printRule("}");
		}
		else if (currentToken().type == "DECREMENT") {
			printRule("decrement {");
			indent++;
			printToken(true);
			expect(currentToken().type);
			printToken(false);
			indent--;
			printRule("}");
		}
		else if (currentToken().type == "lBRACE") parse_functionCall();
		else printToken(false);
	}
	indent--;
	printRule("}");
	indent--;
}

void Parser::parse_subfactor(const string& type) {
	if (currentToken().type == "MULTIPLICATION" || currentToken().type == "DIVISION" || currentToken().type == "MODULUS") {
		indent++;
		printRule("subfactor {");
		indent++;
		expect(currentToken().type);
		printToken(true);
		indent--;
		parse_factor(type);
		parse_subfactor(type);
		printRule("}");
		indent--;
	}
}

void Parser::parse_functionCall() {
	printRule("functionCall {");
	indent++;
	printToken(true);
	expect("lBRACE");
	printToken(true);
	if (currentToken().type == "IDENTIFIER" || literals.count(currentToken().type))
		parse_parameters();
	else if (tokens[cursor].value == "true" || tokens[cursor].value == "false")
		parse_parameters();
	expect("rBRACE");
	printToken(true);
	indent--;
	printRule("}");
}

void Parser::parse_parameters() {
	printRule("parameters {");
	indent++;
	parse_parameter();
	while (currentToken().type == "COMMA") {
		expect(currentToken().type);
		printToken(true);
		parse_parameter();
	}
	indent--;
	printRule("}");
}

void Parser::parse_parameter() {
	printRule("parameter {");
	indent++;
	parse_expression();
	indent--;
	printRule("}");
}