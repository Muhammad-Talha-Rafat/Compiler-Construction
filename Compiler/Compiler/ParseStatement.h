#pragma once
using namespace std;



Node* Parser::parse_statements() {

	try {

		if (peek().value == "++") {

			// it's an pre-increment

			Node* _increment = parse_increment();
			_increment->children.push_back(new Node(expectType("SEMICOLON")));

			return _increment;
		}
		else if (peek().value == "--") {

			// it's a pre-decrement

			Node* _decrement = parse_decrement();
			_decrement->children.push_back(new Node(expectType("SEMICOLON")));

			return _decrement;
		}

		else if (types.count(peek().value) || peek().value == "const" || peek().value == "static") {

			// it's a declaration/definition

			int offset = 0;

			if (peek().value == "const" || peek().value == "static") {
				offset++;

				if (!types.count(peek(offset).value) && peek(offset).value != "void")
					throw ExpectedTypeToken();
			}

			if ((types.count(peek(offset).value))) {

				if (peek(offset + 2).type == "lPARENTHESIS")
					throw SyntaxError("nested function detected ahead");

				Node* variable_node = parse_variable();

				return variable_node;
			}
		}

		else if (peek().type == "IDENTIFIER") {

			// it's an assignment, or post-increment, or post-decrement, or a call statement

			if (peek(1).value == "++") {

				// it's a post-increment

				Node* _increment = parse_increment();
				_increment->children.push_back(new Node(expectType("SEMICOLON")));

				return _increment;
			}
			else if (peek(1).value == "--") {

				// it's a post-decrement

				Node* _decrement = parse_decrement();
				_decrement->children.push_back(new Node(expectType("SEMICOLON")));

				return _decrement;
			}
			else if (peek(1).type == "lPARENTHESIS") {

				// it's a function call statement

				Node* _call_statement = new Node("call_statement");

				_call_statement->children.push_back(new Node(expectType("IDENTIFIER")));
				_call_statement->children.push_back(new Node(expectType("lPARENTHESIS")));

				// validate argumemnts

				if (peek().type != "rPARENTHESIS") {
					Node* arguments_node = parse_arguments();
					if (arguments_node)
						_call_statement->children.push_back(arguments_node);
				}

				_call_statement->children.push_back(new Node(expectType("rPARENTHESIS")));
				_call_statement->children.push_back(new Node(expectType("SEMICOLON")));

				return _call_statement;
			}
			else if (assignment_op.count(peek(1).type)) {

				// it's an assignment

				Node* _assignment = new Node("assignment");

				_assignment->children.push_back(new Node(expectType("IDENTIFIER")));

				if (!assignment_op.count(peek().type)) {
					_assignment->children.push_back(new Node("error", UnexpectedToken("assignment operator", peek()).what()));
					return _assignment;
				}
				_assignment->children.push_back(new Node(consume())); // assignment operator

				Node* expression_node = parse_expression();
				if (expression_node)
					_assignment->children.push_back(expression_node);

				_assignment->children.push_back(new Node(expectType("SEMICOLON")));

				return _assignment;
			}

			// unknown token after IDENTIFIER
			else return new Node("error", SyntaxError("invalid statement starting with \"IDENTIFIER\"").what());
		}

		else if (peek().value == "cout") {

			// it's an output statement

			Node* _cout = new Node("output");

			try {
				_cout->children.push_back(new Node(expectValue("cout")));

				while (peek().type != "SEMICOLON") {
					_cout->children.push_back(new Node(expectType("LEFT_SHIFT")));

					if (peek().value == "endl")
						_cout->children.push_back(new Node(expectValue("endl")));
					else {
						Node* expression_node = parse_expression();

						if (expression_node && expression_node->type == "error")
							throw runtime_error(expression_node->value);

						if (!expression_node)
							throw ExpectedExpression();

						_cout->children.push_back(expression_node);
					}
				}

				_cout->children.push_back(new Node(expectType("SEMICOLON")));
			}
			catch (const runtime_error& e) {
				_cout->children.push_back(new Node("error", e.what()));
			}

			return _cout;
		}

		else if (peek().value == "cin") {

			// it's an input statement

			Node* _cin = new Node("input");

			try {
				_cin->children.push_back(new Node(expectValue("cin")));
				_cin->children.push_back(new Node(expectType("RIGHT_SHIFT")));
				_cin->children.push_back(new Node(expectType("IDENTIFIER")));
				_cin->children.push_back(new Node(expectType("SEMICOLON")));
			}
			catch (const runtime_error& e) {
				_cin->children.push_back(new Node("error", e.what()));
			}

			return _cin;
		}

		else if (peek().value == "for") {

			Node* _for_loop = new Node("for_loop");

			try {
				_for_loop->children.push_back(new Node(expectValue("for")));
				_for_loop->children.push_back(new Node(expectType("lPARENTHESIS")));

				if (types.count(peek().value) || peek().type == "IDENTIFIER") {

					// declaration

					Node* variable_node = new Node("init");

					try {
						if (types.count(peek().value))
							variable_node->children.push_back(new Node(consume())); // type

						variable_node->children.push_back(new Node(expectType("IDENTIFIER")));
						variable_node->children.push_back(new Node(expectType("ASSIGN")));

						Node* expression_node = parse_expression();
						if (expression_node)
							variable_node->children.push_back(expression_node);
					}
					catch (const runtime_error& e) {
						variable_node->children.push_back(new Node("error", e.what()));
					}

					_for_loop->children.push_back(variable_node);

				}
				else throw SyntaxError("expected a declaration or assignment but \"" + peek().value + "\" encountered");

				_for_loop->children.push_back(new Node(expectType("SEMICOLON")));

				// conditions

				Node* conditions_node = parse_conditions();
				if (conditions_node)
					_for_loop->children.push_back(conditions_node);

				_for_loop->children.push_back(new Node(expectType("SEMICOLON")));

				// update

				Node* update_node = new Node("update");

				if (peek().value == "++" || peek().value == "--") {
					try {
						if (peek().value == "++")
							update_node->children.push_back(parse_increment());
						else update_node->children.push_back(parse_decrement());
					}
					catch (const runtime_error& e) {
						update_node->children.push_back(new Node("error", e.what()));
					}
				}
				else if (peek().type == "IDENTIFIER") {

					if (peek(1).value == "++" || peek(1).value == "--") {
						try {
							update_node->children.push_back(peek(1).value == "++" ? parse_increment() : parse_decrement());
						}
						catch (const runtime_error& e) {
							update_node->children.push_back(new Node("error", e.what()));
						}
					}
					else try {
						update_node->children.push_back(new Node(expectType("IDENTIFIER")));

						if (!assignment_op.count(peek().type))
							throw SyntaxError("expected an assignment operator but \"" + peek().value + "\" encountered");

						update_node->children.push_back(new Node(consume()));

						Node* expression_node = parse_expression();
						if (expression_node)
							update_node->children.push_back(expression_node);
					}
					catch (const runtime_error& e) {
						update_node->children.push_back(new Node("error", e.what()));
					}

				}
				else throw SyntaxError("expected an update statement but \"" + peek().value + "\" encountered");

				_for_loop->children.push_back(update_node);

				_for_loop->children.push_back(new Node(expectType("rPARENTHESIS")));

				// block

				_for_loop->children.push_back(new Node(expectType("lBRACE")));

				Node* _block = new Node("block");

				while (peek().type != "rBRACE") {
					_block->children.push_back(parse_statements());
					if (!_block->children.empty() && _block->children.back()->type == "error")
						throw runtime_error(_block->children.back()->value);
				}

				if (!_block->children.empty())
					_for_loop->children.push_back(_block);

				_for_loop->children.push_back(new Node(expectType("rBRACE")));
			}
			catch (const runtime_error& e) {
				_for_loop->children.push_back(new Node("error", e.what()));
			}

			return _for_loop;
		}

		else if (peek().value == "while") {

			// it's a "while" loop

			Node* _while_loop = new Node("while_loop");

			try {

				_while_loop->children.push_back(new Node(expectValue("while")));
				_while_loop->children.push_back(new Node(expectType("lPARENTHESIS")));

				Node* conditions_node = parse_conditions();
				if (conditions_node)
					_while_loop->children.push_back(conditions_node);

				_while_loop->children.push_back(new Node(expectType("rPARENTHESIS")));
				_while_loop->children.push_back(new Node(expectType("lBRACE")));

				// block

				Node* _block = new Node("block");

				while (peek().type != "rBRACE") {
					_block->children.push_back(parse_statements());
					if (_block->children.empty() && _block->children.back()->type == "error")
						throw runtime_error(_block->children.back()->value);
				}

				if (!_block->children.empty())
					_while_loop->children.push_back(_block);

				_while_loop->children.push_back(new Node(expectType("rBRACE")));

			}
			catch (const runtime_error & e) {
				_while_loop->children.push_back(new Node("error", e.what()));
			}

			return _while_loop;
		}
		
		else if (peek().value == "do") {

			// it's a do "while" loop

			Node* _do_while_loop = new Node("do_while_loop");

			try {

				_do_while_loop->children.push_back(new Node(expectValue("do")));
				_do_while_loop->children.push_back(new Node(expectType("lBRACE")));

				// block

				Node* _block = new Node("block");

				while (peek().type != "rBRACE") {
					_block->children.push_back(parse_statements());
					if (_block->children.empty() && _block->children.back()->type == "error")
						throw runtime_error(_block->children.back()->value);
				}

				if (!_block->children.empty())
					_do_while_loop->children.push_back(_block);

				_do_while_loop->children.push_back(new Node(expectType("rBRACE")));
				_do_while_loop->children.push_back(new Node(expectValue("while")));
				_do_while_loop->children.push_back(new Node(expectType("lPARENTHESIS")));

				Node* conditions_node = parse_conditions();
				if (conditions_node)
					_do_while_loop->children.push_back(conditions_node);

				_do_while_loop->children.push_back(new Node(expectType("rPARENTHESIS")));
				_do_while_loop->children.push_back(new Node(expectType("SEMICOLON")));
			}
			catch (const runtime_error & e) {
				_do_while_loop->children.push_back(new Node("error", e.what()));
			}

			return _do_while_loop;
		}

		else if (peek().value == "if") {

			// it's an "if" block, handling "else if" and "else" as well

			Node* _decision = new Node("decision");

			try {

				Node* _if_block = new Node("if_block");

				_if_block->children.push_back(new Node(expectValue("if")));
				_if_block->children.push_back(new Node(expectType("lPARENTHESIS")));

				Node* conditions_node = parse_conditions();
				if (conditions_node)
					_if_block->children.push_back(conditions_node);

				_if_block->children.push_back(new Node(expectType("rPARENTHESIS")));
				_if_block->children.push_back(new Node(expectType("lBRACE")));

				// block

				Node* _block = new Node("block");

				while (peek().type != "rBRACE") {
					_block->children.push_back(parse_statements());
					if (_block->children.empty() && _block->children.back()->type == "error")
						throw runtime_error(_block->children.back()->value);
				}

				if (!_block->children.empty())
					_if_block->children.push_back(_block);

				_if_block->children.push_back(new Node(expectType("rBRACE")));

				_decision->children.push_back(_if_block);

				// handle "else if" and "else"

				while (peek().value == "else if" || peek().value == "else") {

					if (peek().value == "else if") {

						Node* _else_if_block = new Node("else_if_block");

						_else_if_block->children.push_back(new Node(expectValue("else if")));
						_else_if_block->children.push_back(new Node(expectType("lPARENTHESIS")));

						Node* conditions_node = parse_conditions();
						if (conditions_node)
							_else_if_block->children.push_back(conditions_node);

						_else_if_block->children.push_back(new Node(expectType("rPARENTHESIS")));
						_else_if_block->children.push_back(new Node(expectType("lBRACE")));

						// block

						Node* else_if_block = new Node("block");

						while (peek().type != "rBRACE") {
							else_if_block->children.push_back(parse_statements());
							if (else_if_block->children.empty() && else_if_block->children.back()->type == "error")
								throw runtime_error(else_if_block->children.back()->value);
						}

						if (!else_if_block->children.empty())
							_else_if_block->children.push_back(else_if_block);

						_else_if_block->children.push_back(new Node(expectType("rBRACE")));

						_decision->children.push_back(_else_if_block);
					}
					else {
						Node* _else_block = new Node("else_block");

						_else_block->children.push_back(new Node(expectValue("else")));
						_else_block->children.push_back(new Node(expectType("lBRACE")));

						// block

						Node* else_block = new Node("block");

						while (peek().type != "rBRACE") {
							else_block->children.push_back(parse_statements());
							if (else_block->children.empty() && else_block->children.back()->type == "error")
								throw runtime_error(else_block->children.back()->value);
						}

						if (!else_block->children.empty())
							_else_block->children.push_back(else_block);

						_else_block->children.push_back(new Node(expectType("rBRACE")));

						_decision->children.push_back(_else_block);

						break;
					}
				}
			}
			catch (const runtime_error& e) {
				_decision->children.push_back(new Node("error", e.what()));
			}

			return _decision;
		}

		else if (peek().value == "else if" || peek().value == "else") {

			// it's an "else if" or "else" block without an "if" block (error)
			throw SyntaxError("Missing \"if\" block before \"" + peek().value + "\"");

		}	
		
		else if (peek().value == "return") {

			// it's a return statement
			return parse_return();

		}

		else throw SyntaxError("expected a valid statement but \"" + peek().value + "\" encountered");

	}
	catch (const runtime_error& e) {
		Node* error_node = new Node("error", e.what());
		return error_node;
	}

	return nullptr;
}



Node* Parser::parse_return() {

	Node* _return = new Node("return");

	try {
		_return->children.push_back(new Node(consume()));

		Node* expression_node = parse_expression();
		if (expression_node)
			_return->children.push_back(expression_node);

		_return->children.push_back(new Node(expectType("SEMICOLON")));
	}
	catch (const runtime_error& e) {
		_return->children.push_back(new Node("error", e.what()));
	}

	return _return;
}



Node* Parser::parse_increment() {

	Node* _increment = new Node("increment");

	try {
		if (peek().value == "++") {
			// pre-increment
			_increment->children.push_back(new Node(consume()));
			_increment->children.push_back(new Node(expectType("IDENTIFIER")));
		}
		else {
			// post-increment
			_increment->children.push_back(new Node(expectType("IDENTIFIER")));
			_increment->children.push_back(new Node(expectValue("++")));
		}
	}
	catch (const runtime_error& e) {
		_increment->children.push_back(new Node("error", e.what()));
	}

	return _increment;
}



Node* Parser::parse_decrement() {

	Node* _decrement = new Node("decrement");

	try {
		if (peek().value == "--") {
			// pre-decrement
			_decrement->children.push_back(new Node(consume()));
			_decrement->children.push_back(new Node(expectType("IDENTIFIER")));
		}
		else {
			// post-decrement
			_decrement->children.push_back(new Node(expectType("IDENTIFIER")));
			_decrement->children.push_back(new Node(expectValue("--")));
		}
	}
	catch (const runtime_error& e) {
		_decrement->children.push_back(new Node("error", e.what()));
	}

	return _decrement;
}



Node* Parser::parse_conditions() {

	Node* _conditions = new Node("conditions");

	try {
		Node* left_node = nullptr;

		if (peek().type == "lPARENTHESIS") {
			_conditions->children.push_back(new Node(expectType("lPARENTHESIS")));

			Node* conditions_node = parse_conditions();
			if (conditions_node)
				_conditions->children.push_back(conditions_node);

			_conditions->children.push_back(new Node(expectType("rPARENTHESIS")));
		}
		else {
			left_node = parse_comparison();
			if (left_node)
				_conditions->children.push_back(left_node);
		}

		while (logical_op.count(peek().type)) {

			_conditions->children.push_back(new Node(consume())); // AND_LOGIC / OR_LOGIC

			if (peek().type == "lPARENTHESIS") {
				_conditions->children.push_back(new Node(expectType("lPARENTHESIS")));

				Node* conditions_node = parse_conditions();
				if (conditions_node)
					_conditions->children.push_back(conditions_node);

				_conditions->children.push_back(new Node(expectType("rPARENTHESIS")));
			}
			else {
				Node* right_node = parse_comparison();
				if (right_node)
					_conditions->children.push_back(right_node);
			}
		}
	}
	catch (const runtime_error& e) {
		_conditions->children.push_back(new Node("error", e.what()));
	}

	return _conditions;
}



Node* Parser::parse_comparison() {

	Node* _comparison = new Node("comparison");

	try {
		Node* left_node = parse_expression();
		if (left_node)
			_comparison->children.push_back(left_node);

		if (!comparison_op.count(peek().type))
			throw SyntaxError("expected a comparison operator but \"" + peek().value + "\" encountered");

		_comparison->children.push_back(new Node(consume())); // comparison operator

		Node* right_node = parse_expression();
		if (right_node)
			_comparison->children.push_back(right_node);
	}
	catch (const runtime_error& e) {
		_comparison->children.push_back(new Node("error", e.what()));
	}

	return _comparison;
}

/*


a < 0 && (b < 0 || c > 0)

conditions
	comparison
		IDENTIFIER: a
		LESS_THAN: <
		INTEGER: 0
	AND_LOGIC: &&
	lPARENTHESIS: (
	conditions
		comparison
			IDENTIFIER: b
			LESS_THAN: <
			INTEGER: 0
		OR_LOGIC: ||
		comparison
			IDENTIFIER: c
			LESS_THAN: >
			INTEGER: 0
	rPARENTHESIS: )



if (r < 0) {}
else if (r > 0) {}
else {}

decision_tree
	if_block
		KEYWORD: if
		lPARENTHESIS: (
		conditions
			comparison
				IDENTIFIER: r
				SMALLER_THAN: <
				INTEGER: 0
		rPARENTHESIS: )
		lBRACE: {
		rBRACE: }
	else_if_block
		KEYWORD: else if
		lPARENTHESIS: (
		conditions
			comparison
				IDENTIFIER: r
				GREATER_THAN: >
				INTEGER: 0
		rPARENTHESIS: )
		lBRACE: {
		rBRACE: }
	else_block
		KEYWORD: else
		lBRACE: {
		rBRACE: }
		




*/