#pragma once
using namespace std;


void Parser::parse_object() {
	indent++;
	printRule(currentToken().value == "class" ? "class {" : "struct {");
	indent++;
	expect("KEYWORD"); // class / struct
	printToken(true);
	expect("IDENTIFIER");
	printToken(true);
	expect("lPARENTHESIS");
	printToken(true);
	parse_objBlock();
	expect("rPARENTHESIS");
	printToken(true);
	expect("SEMICOLON");
	printToken(true);
	indent--;
	printRule("}");
	indent--;
}


void Parser::parse_objBlock() {
	while (currentToken().value == "private" || currentToken().value == "public" || currentToken().value == "protected") {
		printRule("block {");
		indent++;
		printRule("access {");
		indent++;
		expect(currentToken().type);
		printToken(true);
		expect("COLON");
		printToken(false);
		indent--;
		printRule("}");
		printRule("members {");
		while (types.count(currentToken().value) || currentToken().value == "const" || currentToken().value == "static" || currentToken().value == "void")
			parse_declare();
		printRule("}");
		indent--;
		printRule("}");
	}
	if (currentToken().value != "private" && currentToken().value != "public" && currentToken().value != "protected" && tokens[cursor - 1].value == "{")
		throw runtime_error("Syntax error: 'access' keyword expected");
}




/*

class {
	KEYWORD: class,
	IDENTIFIER: fraction,
	lPARENTHESIS: {,
	block {
		access {
			KEYWORD: private,
			COLON: :
		}
		members {
			member {
				KEYWORD: int,
				IDENTIFIER: numerator,
				SEMICOLON: ;
			}
			member {
				KEYWORD: int,
				IDENTIFIER: denominator,
				SEMICOLON: ;
			}
		}
	}
	block {
		access {
			KEYWORD: public,
			COLON: :
		}
		members {
			member {
				KEYWORD: void,
				IDENTIFIER: print,
				lBRACE: (,
				rBRACE: ),
				lPARENTHESIS: {,
				statements {
					statement {
						output {
							KEYWORD: cout,
							LEFTSHIFT: <<,
							expression {
								term {
									factor {
										IDENTIFIER: numerator
									}
								}
							}
							LEFTSHIFT: <<,
							CHARACTER: "/",
							LEFTSHIFT: <<,
							expression {
								term {
									factor {
										IDENTIFIER: denominator
									}
								}
							}
							LEFTSHIFT: <<,
							KEYWORD: endl,
							SEMICOLON: ;
						}
					}
				}
				rPARENTHESIS: },
			}
		}
	}
	rPARENTHESIS: },
	SEMICOLON: ;,
}

*/