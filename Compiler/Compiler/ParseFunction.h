#pragma once
using namespace std;



Node* Parser::parse_function(const string& type, const string& name) {

	Node* _function = new Node("function");
	bool return_trigger = false;

	try {

		if (peek().value == "const" || peek().value == "static") {
			if (type == "void")
				throw SyntaxError("\"void\" function can't be '" + peek().value + "'");

			if (type == "main")
				throw SyntaxError("\"main\" function can't be '" + peek().value + "'");

			_function->children.push_back(new Node(consume()));
		}

		_function->children.push_back(new Node(consume()));		// type
		_function->children.push_back(new Node(consume()));		// IDENTIFIER

		_function->children.push_back(new Node(expectType("lPARENTHESIS")));

		// validate parameters

		if (peek().type != "rPARENTHESIS" && name == "main")
			throw SyntaxError("\"main\" function can't have any parameters");

		while (peek().type != "rPARENTHESIS") {

			Node* parameter_node = parse_parameters();

			if (parameter_node)
				_function->children.push_back(parameter_node);

			if (peek().type == "COMMA")
				_function->children.push_back(new Node(consume()));
		}

		_function->children.push_back(new Node(expectType("rPARENTHESIS")));
		_function->children.push_back(new Node(expectType("lBRACE")));

		// now comes the body

		Node* _block = new Node("block");

		while (peek().type != "rBRACE") {

			Node* statements_node = parse_statements();

			if (statements_node) {

				if ((!statements_node->children.empty() && statements_node->children.back()->type == "error") || statements_node->type == "error") {
					_block->children.push_back(statements_node);
					_function->children.push_back(_block);
					return _function;
				}

				if (statements_node)
					_block->children.push_back(statements_node);

				if (statements_node->type == "return")
					return_trigger = true;

				if (cursor < tokens.size() && peek().value == "return") {
					if (type == "void")
						throw SyntaxError("\"void\" function can not return");

					Node* return_node = parse_return();
					if (return_node)
						_block->children.push_back(return_node);

					return_trigger = true;
				}
			}
		}

		if (!_block->children.empty())
			_function->children.push_back(_block);

		if (name != "main" && type != "void" && !return_trigger)
			throw MissingReturn();

		_function->children.push_back(new Node(expectType("rBRACE")));
	}
	catch (const runtime_error& e) {
		_function->children.push_back(new Node("error", e.what()));
	}

	return _function;
}



Node* Parser::parse_parameters() {

	Node* _parameter = new Node("parameter");

	if (peek().value == "const" || peek().value == "static")
		_parameter->children.push_back(new Node(consume()));

	if (types.count(peek().value) == 0)
		throw ExpectedTypeToken();

	_parameter->children.push_back(new Node(consume())); // type
	_parameter->children.push_back(new Node(expectType("IDENTIFIER")));

	return _parameter;
}





