#pragma once
using namespace std;



Node* Parser::parse_declarations() {

	Node* _declarations = new Node("declarations");

	try {
		while (cursor < tokens.size()) {

			if (peek().value == "#define") {
				Node* define_node = parse_define();
				if (define_node)
					_declarations->children.push_back(define_node);
			}
			else if (peek().value == "const" || peek().value == "static" || types.count(peek().value) || peek().value == "void") {

				int offset = 0;

				if (peek().value == "const" || peek().value == "static") {
					offset++;

					if (!types.count(peek(offset).value) && peek(offset).value != "void")
						throw ExpectedTypeToken();
				}


				if ((types.count(peek(offset).value) || peek(offset).value == "void") && (peek(offset + 1).type == "IDENTIFIER" || peek(offset + 1).value == "main")) {

					if (peek(offset + 2).type == "lPARENTHESIS") {

						// It's a function declaration/definition

						string type = peek(offset).value;
						string name = peek(offset + 1).value;

						if (main_trigger && name == "main")
							throw SyntaxError("\"main\" function already exists");


						if (type == "int" && name == "main")
							main_trigger = true;
						else if (type != "int" && name == "main")
							throw SyntaxError("\"main\" function can only be type \"int\"");

						Node* function_node = parse_function(type, name);

						if (!function_node->children.empty() && function_node->children.back()->type == "error") {
							_declarations->children.push_back(function_node);
							throw runtime_error(function_node->children.back()->value);
						}

						_declarations->children.push_back(function_node);

					}
					else if (peek(offset + 2).type == "ASSIGN" || peek(offset + 2).type == "COMMA" || peek(offset + 2).type == "SEMICOLON") {

						// it's a variable declaration

						Node* variable_node = parse_variable();
						if (!variable_node->children.empty() && variable_node->children.back()->type == "error") {
							_declarations->children.push_back(variable_node);
							return _declarations;
						}

						_declarations->children.push_back(variable_node);
					}
					else throw SyntaxError("expected a declaration but \"" + peek(offset + 2).type + "\" encountered");
				}
				else throw SyntaxError("invalid declaration starting with \"" + peek().value + "\"");
			}

			else if (peek().value == "class" || peek().value == "struct") {

				// it's an object declaration

				Node* object_node = parse_object();
				if (!object_node->children.empty() && object_node->children.back()->type == "error") {
					_declarations->children.push_back(object_node);
					return _declarations;
				}

				_declarations->children.push_back(object_node);
			}
			else throw SyntaxError("invalid token \"" + peek().value + "\" to start a declaration");
		}
	}
	catch (const runtime_error& e) {
		_declarations->children.push_back(new Node("error", e.what()));
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

	try {
		bool assigned = false;

		if (peek().value == "const" || peek().value == "static")
			_variable->children.push_back(new Node(consume()));

		if (!types.count(peek().value))
			throw ExpectedTypeToken();

		_variable->children.push_back(new Node(consume()));						// type

		if (_variable->children.front()->value == "void" || (_variable->children.size() > 1 && _variable->children[1]->value == "void"))
			throw SyntaxError("variables can't have type \"void\"");

		_variable->children.push_back(new Node(expectType("IDENTIFIER")));		// IDENTIFIER

		if (peek().type == "ASSIGN") {
			_variable->children.push_back(new Node(consume()));					// ASSIGN

			Node* expression_node = parse_expression();
			if (expression_node)
				_variable->children.push_back(expression_node);

			assigned = true;
		}
		else if (peek().type == "COMMA") while (peek().type == "COMMA") {
			_variable->children.push_back(new Node(consume()));					// COMMA
			_variable->children.push_back(new Node(consume()));					// IDENTIFIER
		}
		else if (peek().type != "SEMICOLON") {
			_variable->children.push_back(new Node("error", UnexpectedToken("ASSIGN", peek()).what()));
			return _variable;
		}

		// a const/static variable with no value at initialization
		if ((_variable->children[0]->value == "const" || _variable->children[0]->value == "static") && !assigned)
			_variable->children.push_back(new Node("error", "Runtime error: const/static variable must have a value"));

		_variable->children.push_back(new Node(expectType("SEMICOLON")));						// SEMICOLON
	}
	catch (const runtime_error& e) {
		_variable->children.push_back(new Node("error", e.what()));
	}

	return _variable;
}



Node* Parser::parse_expression() {

	Node* left = parse_term();

	if (left && left->type == "error")
		return left;

	if (!add_sub_op.count(peek().type))
		return left;

	Node* _expression = new Node("expression");
	_expression->children.push_back(left);

	while (add_sub_op.count(peek().type)) {
		_expression->children.push_back(new Node(consume()));	// ADD/SUB
		
		Node* term_node = parse_term();
		if (term_node && term_node->type == "error")
			return term_node;

		_expression->children.push_back(term_node);
	}

	return _expression;
}



Node* Parser::parse_term() {

	Node* _left = parse_factor();

	if (_left && _left->type == "error")
		return _left;

	if (!mul_div_mod_op.count(peek().type))
		return _left;

	Node* term = new Node("expression");
	term->children.push_back(_left);

	while (mul_div_mod_op.count(peek().type)) {
		term->children.push_back(new Node(consume()));			// MUL/DIV/MOD
		
		Node* factor_node = parse_factor();
		if (factor_node && factor_node->type == "error")
			return factor_node;
		
		term->children.push_back(factor_node);
	}

	return term;
}



Node* Parser::parse_factor() {

	if (cursor >= tokens.size())
		return new Node("error", UnexpectedEOF().what());

	// parenthesis enclosed expressions
	if (peek().type == "lPARENTHESIS") {
		Node* _expression = new Node("expression");
		_expression->children.push_back(new Node(expectType("lPARENTHESIS")));
		
		Node* expression_node = parse_expression();
		if (expression_node && expression_node->type == "error")
			return expression_node;

		_expression->children.push_back(expression_node);

		try {
			_expression->children.push_back(new Node(expectType("rPARENTHESIS")));
		}
		catch (const runtime_error& e) {
			return new Node("error", e.what());
		}

		return _expression;
	}

	// increment or decrement
	if (peek().value == "++")
		return parse_increment();
	if (peek().value == "--")
		return parse_decrement();

	// identifier or function call
	if (peek().type == "IDENTIFIER") {
		if (cursor + 1 < tokens.size() && peek(1).type == "lPARENTHESIS")
			return parse_functionCall();
		else return new Node(consume());
	}

	// literal
	if (literals.count(peek().type) || peek().value == "true" || peek().value == "false") {
		if (cursor + 1 < tokens.size() && peek(1).type == "IDENTIFIER")
			return new Node("error", SyntaxError("expected an operator but \"" + peek(1).type + "\" encountered").what());

		return new Node(consume());
	}

	// unexpected token
	return new Node("error", UnexpectedToken("expression", peek()).what());
}



Node* Parser::parse_functionCall() {

	Node* _function_call = new Node("function_call");

	try {
		_function_call->children.push_back(new Node(consume())); // IDENTIIFER
		_function_call->children.push_back(new Node(expectType("lPARENTHESIS")));

		if (peek().type != "rPARENTHESIS") {
			Node* arguments_node = parse_arguments();
			if (arguments_node) {
				if (!arguments_node->children.empty())
					for (auto argument_node : arguments_node->children)
						if (argument_node && argument_node->type == "error")
							throw runtime_error(argument_node->value);
				_function_call->children.push_back(arguments_node);
			}
		}

		_function_call->children.push_back(new Node(expectType("rPARENTHESIS")));
	}
	catch (const runtime_error& e) {
		_function_call->children.push_back(new Node("error", e.what()));
	}

	return _function_call;
}



Node* Parser::parse_arguments() {

	Node* _arguments = new Node("arguments");

	while (peek().type != "rPARENTHESIS") {

		Node* expression_node = parse_expression();

		if (expression_node) {

			if (expression_node->type == "error") {
				_arguments->children.push_back(expression_node);

				while (peek().type != "COMMA" && peek().type != "rPARENTHESIS")
					ignore();

				if (peek().type == "rPARENTHESIS")
					break;
			}
			else _arguments->children.push_back(expression_node);

			if (peek().type == "COMMA")
				_arguments->children.push_back(new Node(consume()));
			else if (peek().type != "rPARENTHESIS") {
				_arguments->children.push_back(new Node("error", UnexpectedToken("rPARENTHESIS", peek()).what()));
				break;
			}
		}
		else break;
	}

	return _arguments;
}