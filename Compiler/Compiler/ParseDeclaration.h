#pragma once
using namespace std;



Node* Parser::parse_declarations() {

	Node* _declarations = new Node("declarations");

	while (cursor < tokens.size()) {

		if (peek().value == "#define") {
			Node* define_node = parse_define();
			if (define_node)
				_declarations->children.push_back(define_node);
		}
		else if (peek().value == "const" || peek().value == "static" || types.count(peek().value) || peek().value == "void") {

			int offset = 0;

			if (peek().value == "const" || peek().value == "static")
				offset++;

			if (types.count(peek(offset).value) && peek(offset + 1).type == "IDENTIFIER") {
				if (peek(offset + 2).type == "lPARENTHESIS") {
					// It's a function declaration/definition
					// parseFunction() yet to be implemented
				}
				else {
					// it's a variable declaration
					Node* variable_node = parse_variable();
					if (variable_node)
						_declarations->children.push_back(variable_node);
				}
			}
			else {
				Node* variable_node = new Node("variable"); // only for the sake of printing
				variable_node->children.push_back(new Node(consume()));

				if (!types.count(peek().value))
					variable_node->children.push_back(new Node("error", UnexpectedToken("type", peek()).what()));
				else {
					variable_node->children.push_back(new Node(consume()));
					variable_node->children.push_back(new Node("error", UnexpectedToken("IDENTIIFER", peek()).what()));
				}

				_declarations->children.push_back(variable_node);
				return _declarations;
			}
		}
		else break;
	}

	if (_declarations->children.empty())
		return nullptr;

	return _declarations;
}



Node* Parser::parse_define() {

	Node* _define = new Node("define");

	try {

		_define->children.push_back(new Node(expectValue("#define")));
		_define->children.push_back(new Node(expectType("IDENTIFIER")));

		if (literals.count(peek().type) || literals.count(peek().value))
			_define->children.push_back(new Node(consume()));
		else throw UnexpectedToken("value", peek());

	}
	catch (const runtime_error& e) {
		_define->children.push_back(new Node("error", e.what()));
	}

	return _define;
}



Node* Parser::parse_variable() {

	Node* _variable = new Node("variable");
	bool assigned = false;

	if (peek().value == "const" || peek().value == "static")
		_variable->children.push_back(new Node(consume()));

	string type = peek().value;
	_variable->children.push_back(new Node(consume()));		// type
	_variable->children.push_back(new Node(consume()));		// IDENTIFIER

	if (peek().type == "ASSIGN") {
		_variable->children.push_back(new Node(consume()));	// ASSIGN

		Node* expression_node = parse_expression(type);
		if (expression_node)
			_variable->children.push_back(expression_node);

		assigned = true;
	}
	else while (peek().type == "COMMA") {
		_variable->children.push_back(new Node(consume()));	// COMMA
		_variable->children.push_back(new Node(consume()));	// IDENTIFIER
	}

	// a const/static variable with no value at initialization
	if ((_variable->children[0]->value == "const" || _variable->children[0]->value == "static") && !assigned)
		_variable->children.push_back(new Node("error", "Runtime error: const/static variable must have a value"));

	_variable->children.push_back(new Node(consume()));		// SEMICOLON

	return _variable;
}



Node* Parser::parse_expression(const string& type) {
	Node* left = parse_term(type);

	if (!add_sub_op.count(peek().type))
		return left;

	Node* expr = new Node("expression");
	expr->children.push_back(left);

	while (add_sub_op.count(peek().type)) {
		expr->children.push_back(new Node(consume()));   // + or -
		expr->children.push_back(parse_term(type));
	}
	return expr;
}


Node* Parser::parse_term(const string& type) {
	Node* left = parse_factor(type);

	if (!mul_div_mod_op.count(peek().type))
		return left;

	Node* term = new Node("expression"); // can keep 'expression' if uniform
	term->children.push_back(left);

	while (mul_div_mod_op.count(peek().type)) {
		term->children.push_back(new Node(consume()));   // *, /, %
		term->children.push_back(parse_factor(type));
	}
	return term;
}


Node* Parser::parse_factor(const string& type) {
	// ( expression )
	if (peek().type == "lPARENTHESIS") {
		Node* expr = new Node("expression");
		expr->children.push_back(new Node(consume())); // (
		expr->children.push_back(parse_expression(type));
		expr->children.push_back(new Node(expectType("rPARENTHESIS"))); // )
		return expr;
	}

	// identifier or function call
	if (peek().type == "IDENTIFIER") {
		return new Node(consume());
	}

	// literal (int, float, string, bool, char)
	if (literals.count(peek().type) || peek().value == "true" || peek().value == "false") {
		return new Node(consume());
	}

	// unexpected token
	return new Node("error", UnexpectedToken("expression", peek()).what());
}





/*
variable							for int a = 7;
	KEYWORD: int
	IDENTIFIER: a
	ASSIGN: =
	INTEGER: 7
	SEMICOLON: ;
variable							for int a = b;
	KEYWORD: int
	IDENTIFIER: a
	ASSIGN: =
	IDENTIFIER: b
	SEMICOLON: ;
variable							for int a = x + y;
	KEYWORD: int
	IDENTIFIER: a
	ASSIGN: =
	IDENTIFIER: x
	ADDITION: +
	IDENTIFIER: y
	SEMICOLON: ;
variable							for int a = (x + y);
	KEYWORD: int
	IDENTIFIER: a
	ASSIGN: =
	expression
		lPARENTHESIS: (
		IDENTIFIER: x
		ADDITION: +
		IDENTIFIER: y
		rPARENTHESIS: )
	SEMICOLON: ;
variable							for int a = b + (c * 2);
	KEYWORD: int
	IDENTIFIER: a
	ASSIGN: =
	expression
		IDENTIFIER: b
		ADDITION: +
		expression:
			lPARENTHESIS: (
			IDENTIFIER: c
			MULTIPLICATION: *
			INTEGER: 2
			rPARENTHESIS: )
	SEMICOLON: ;
variable							for int a = (x + y) * (x - y % 2);
	KEYWORD: int
	IDENTIFIER: a
	ASSIGN: =
	expression
		expression
			lPARENTHESIS: (
			IDENTIFIER: x
			ADDITION: +
			IDENTIFIER: y
			rPARENTHESIS: )
		MULTIPLICATION: *
		expression:
			lPARENTHESIS: (
			IDENTIFIER: x
			SUBRACTION: -
			IDENTIFIER: y
			MODULUS: %
			INTEGER: 2
			rPARENTHESIS: )
	SEMICOLON: ;
variable							for int a = calculate()
	KEYWORD: int
	IDENTIFIER: a
	ASSIGN: =
	function call
		IDENTIFIER: calculate
		lPARENTHESIS: (
		rPARENTHESIS: )
	SEMICOLON: ;
variable							for int a = calculate(average, 9 + 11);
	KEYWORD: int
	IDENTIFIER: a
	ASSIGN: =
	function call
		IDENTIFIER: calculate
		lPARENTHESIS: (
		parameters:
			IDENTIIFER: average
			COMMA: ,
			INTEGER: 9
			ADDITION: +
			INTEGER: 11
		rPARENTHESIS: )
	SEMICOLON: ;
*/







/*

Node* Parser::parse_declare() {

	Node* declare_node = new Node("declare");
	string type;

	if (currentToken().value == "const" || currentToken().value == "static")
		declare_node->children.push_back(new Node(expect(currentToken().type)));

	if (types.count(currentToken().value)) {

		type = currentToken().value;
		declare_node->children.push_back(new Node(expect(currentToken().type)));

	}
	else if (currentToken().value == "void") {

		Node* voidfunction_node = parse_voidfunction();
		declare_node->children.push_back(voidfunction_node);

		// "declarations" block: complete
		goto end;

	}
	else throw ExpectedTypeToken();

	if (currentToken() == token{ "KEYWORD", "main" }) {

		this->main = true;

		Node* mainfunction_node = parse_mainfunction();
		declare_node->children.push_back(mainfunction_node);

		// "declarations" block: complete
		goto end;

	}

	declare_node->children.push_back(new Node(expect("IDENTIFIER")));

	if (currentToken().type == "ASSIGN") {

		Node* assign_node = new Node("assignment");
		assign_node->children.push_back(new Node(expect("ASSIGN").value));

		Node* expression_node = parse_expression(type);
		if (expression_node)
			assign_node->children.push_back(expression_node);

		declare_node->children.push_back(new Node(expect("SEMICOLON")));
		declare_node->children.push_back(assign_node);

	}
	else if (currentToken().type == "lBRACE") {
		Node* function_node = parse_function(type);
		declare_node->children.push_back(function_node);
	}
	else if (currentToken().type == "SEMICOLON")
		declare_node->children.push_back(new Node(expect(currentToken().type)));
	else if (currentToken().type != "ASSIGN" && currentToken().type != "SEMICOLON")
		throw UnexpectedToken(";", tokens[cursor]);
	
end:
	
	return declare_node;

}



Node* Parser::parse_expression(const string& type = "") {

	Node* expression = new Node("expression");

	Node* term_node = parse_term(type);
	if (term_node)
		expression->children.push_back(term_node);

	Node* subterm_node = parse_subterm(type);
	if (subterm_node)
		expression->children.push_back(subterm_node);

	return expression;
}



Node* Parser::parse_term(const string& type) {

	Node* term = new Node("term");

	Node* factor_node = parse_factor(type);
	if (factor_node)
		term->children.push_back(factor_node);

	Node* subfactor_node = parse_subfactor(type);
	if (subfactor_node)
		term->children.push_back(subfactor_node);

	return term;
}



Node* Parser::parse_subterm(const string& type) {

	if (add_sub_op.count(currentToken().type)) {

		Node* subterm = new Node("subfactor");
		subterm->children.push_back(new Node(expect(currentToken().type)));

		Node* term_node = parse_term(type);
		if (term_node)
			subterm->children.push_back(term_node);

		Node* subterm_node = parse_subterm(type);
		if (subterm_node)
			subterm->children.push_back(subterm_node);

		return subterm;
	}

	return nullptr;
}



Node* Parser::parse_factor(const string& type) {

	Node* factor = new Node("factor");

	if (currentToken().type == "lBRACE") {
		factor->children.push_back(new Node(expect("lBRACE")));

		Node* expression_node = parse_expression(type);
		if (expression_node)
			factor->children.push_back(expression_node);

		factor->children.push_back(new Node(expect("rBRACE")));
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
		else factor->children.push_back(new Node(expect(currentToken().type)));

	}
	else if (currentToken().type == "INCREMENT" || currentToken().type == "DECREMENT") {

		factor->children.push_back(new Node(expect(currentToken().type)));
		factor->children.push_back(new Node(expect("IDENTIFIER")));

	}
	else if (currentToken().type == "IDENTIFIER") {

		factor->children.push_back(new Node(expect("IDENTIFIER")));

		if (currentToken().type == "INCREMENT" || currentToken().type == "DECREMENT")
			factor->children.push_back(new Node(expect(currentToken().type)));

		else if (currentToken().type == "lBRACE") {
			Node* functioncall_node = parse_functionCall();
			if (functioncall_node)
				factor->children.push_back(functioncall_node);
		}
	}

	return factor;
}



Node* Parser::parse_subfactor(const string& type) {

	if (mul_div_mod_op.count(currentToken().type)) {

		Node* subfactor = new Node("subfactor");

		subfactor->children.push_back(new Node(expect(currentToken().type)));

		Node* factor_node = parse_factor(type);
		if (factor_node)
			subfactor->children.push_back(factor_node);

		Node* subfactor_node = parse_subfactor(type);
		if (subfactor_node)
			subfactor->children.push_back(subfactor_node);

		return subfactor;
	}

	return nullptr;
}



Node* Parser::parse_functionCall() {

	Node* function_call = new Node("functionCall");

	function_call->children.push_back(new Node(expect("lBRACE")));

	if (currentToken().type == "IDENTIFIER" || literals.count(currentToken().type) || tokens[cursor].value == "true" || tokens[cursor].value == "false") {
		Node* parameters_node = parse_parameters();
		if (parameters_node)
			function_call->children.push_back(parameters_node);
	}

	function_call->children.push_back(new Node(expect("rBRACE")));

	return function_call;
}



Node* Parser::parse_parameters() {

	Node* parameters = new Node("parameters");

	Node* parameter_node = parse_parameter();
	if (parameter_node)
		parameters->children.push_back(parameter_node);

	while (currentToken().type == "COMMA") {

		parameters->children.push_back(new Node(expect("COMMA")));

		parameter_node = parse_parameter();
		if (parameter_node)
			parameters->children.push_back(parameter_node);

	}

	return parameters;
}



Node* Parser::parse_parameter() {

	Node* parameter = new Node("parameter");

	Node* expression_node = parse_expression();
	if (expression_node)
		parameter->children.push_back(expression_node);

	return parameter;
}
*/