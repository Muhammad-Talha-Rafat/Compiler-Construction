#pragma once
using namespace std;




Node* Parser::parse_object() {

	Node* _object = new Node(peek().value);

	try {
		_object->children.push_back(new Node(consume())); // class / struct
		_object->children.push_back(new Node(expectType("IDENTIFIER")));
		_object->children.push_back(new Node(expectType("lBRACE")));

		while (peek().type != "rBRACE") {
			Node* objBlock_node = parse_objBlock();

			if (objBlock_node) {
				_object->children.push_back(objBlock_node);

				if (objBlock_node->type == "error") {
					_object->children.push_back(objBlock_node);
					throw runtime_error(objBlock_node->value);
				}

				if (!objBlock_node->children.empty() && objBlock_node->children.back()->type == "error") {
					_object->children.push_back(objBlock_node);
					throw runtime_error(objBlock_node->children.back()->value);
				}
			}
		}

		_object->children.push_back(new Node(expectType("rBRACE")));
		_object->children.push_back(new Node(expectType("SEMICOLON")));
	}
	catch (const runtime_error& e) {
		_object->children.push_back(new Node("error", e.what()));
	}

	return _object;
}



Node* Parser::parse_objBlock() {

	set<string> access = { "private", "public", "protected" };

	if (!access.count(peek().value))
		return new Node("error", ParseError::UnexpectedToken("access specifier", peek()).what());

	Node* access_node = new Node(peek().value + "_block");

	try {
		access_node->children.push_back(new Node(consume())); // private / public / protected
		access_node->children.push_back(new Node(expectType("COLON")));

		while (peek().type != "rBRACE" && !access.count(peek().value)) {

			int offset = 0;

			if (peek().type == "const" || peek().type == "static")
				offset++;

			if ((types.count(peek(offset).value) || peek(offset).value == "void") && (peek(offset + 1).type == "IDENTIFIER" || peek(offset + 1).value == "main")) {

				if (peek(offset + 2).type == "lPARENTHESIS") {

					// It's a function declaration/definition

					string type = peek(offset).value;
					string name = peek(offset + 1).value;

					if (name == "main")
						throw ParseError::SyntaxError("an object can't have \"main\" function");

					Node* function_node = parse_function(type, name);

					if (!function_node->children.empty() && function_node->children.back()->type == "error") {
						access_node->children.push_back(function_node);
						throw runtime_error(function_node->children.back()->value);
					}

					access_node->children.push_back(function_node);

				}
				else if (peek(offset + 2).type == "ASSIGN" || peek(offset + 2).type == "COMMA" || peek(offset + 2).type == "SEMICOLON") {

					// it's a variable declaration

					Node* variable_node = parse_variable();
					if (!variable_node->children.empty() && variable_node->children.back()->type == "error") {
						access_node->children.push_back(variable_node);
						return access_node;
					}

					access_node->children.push_back(variable_node);
				}
				else throw ParseError::SyntaxError("expected a declaration but \"" + peek(offset + 2).type + "\" encountered");
			}
			else throw ParseError::SyntaxError("invalid declaration starting with \"" + peek().value + "\"");
		}

	}
	catch (const runtime_error& e) {
		access_node->children.push_back(new Node("error", e.what()));
	}

	return access_node;
}





/*



class
	KEYWORD: class							class Integer {
	IDENTIFIER: Integer						private:
	lPARENTHESIS: (								int number;
	private_block							public:
		KEYWORD: private						void print() {
		COLON: :									cout << "Integer: " << number << endl;
		variable								}
			KEYWORD: int					};
			IDENTIFIER: number
			SEMICOLON: ;
	public_block
		KEYWORD: public
		COLON: :
		function
			KEYWORD: void
			IDENTIFIER: print
			lPARENTHESIS: (
			rPARENTHESIS: )
			lBRACE: {
			block
				output
					KEYWORD: cout
					LEFT_SHIFT: <<
					STRLITERAL: "Integer: "
					LEFT_SHIFT: <<
					IDENTIFIER: number
					LEFT_SHIFT: <<
					KEYWORD: endl
					SEMICOLON: ;
			rBRACE: }
	rBRACE: }
	SEMICOLON: ;












*/