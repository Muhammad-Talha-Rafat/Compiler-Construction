#pragma once
using namespace std;


void Parser::parse_object() {
	indent++;
	printRule("object {");
	indent++;
	expect("KEYWORD"); // class / struct
	printToken(true);
	expect("IDENTIFIER");
	printToken(true);
	expect("lPARENTHESIS");
	printToken(true);
	parse_objBody();
	expect("rPARENTHESIS");
	printToken(true);
	expect("SEMICOLON");
	printToken(true);
	indent--;
	printRule("}");
	indent--;
}

void Parser::parse_objBody() {
	if (currentToken().type == "KEYWORD" && (currentToken().value == "private" || currentToken().value == "public" || currentToken().value == "protected")) {
		printRule("objBody {");
		parse_objBlock();
		parse_objBody();
		printRule("}");
	}
}

void Parser::parse_objBlock() {
	indent++;
	printRule("objBlock {");
	if (currentToken().value == "private" || currentToken().value == "public" || currentToken().value == "protected") {
		currentToken().type = "ACCESS";
		expect(currentToken().type);
	}
	indent++;
	printToken(true);
	expect("COLON");
	//parse_members();
	printToken(true);
	indent--;
	printRule("}");
	indent--;
}

