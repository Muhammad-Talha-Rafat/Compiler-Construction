#pragma once
using namespace std;



Node* Parser::parse_object() {

	Node* object = new Node(peek().value == "class" ? "class" : "struct");

	try {
		object->children.push_back(new Node(consume())); // class / struct
		object->children.push_back(new Node(expectType("IDENTIFIER")));
		object->children.push_back(new Node(expectType("lBRACE")));

		Node* objBlock_node = parse_objBlock();
		if (objBlock_node)
			object->children.push_back(objBlock_node);

		object->children.push_back(new Node(expectType("rBRACE")));
		object->children.push_back(new Node(expectType("SEMICOLON")));
	}
	catch (const runtime_error& e) {
		return new Node("error", e.what());
	}

	return object;
}



Node* Parser::parse_objBlock() {

	Node* objBlock = new Node("objBlock");

	try {

		while (peek().value == "private" || peek().value == "public" || peek().value == "protected") {

			objBlock->children.push_back(new Node(consume())); // access keyword
			objBlock->children.push_back(new Node(expectType("COLON")));

			while (types.count(currentToken().value) || currentToken().value == "const" || currentToken().value == "static" || currentToken().value = "void") {
				Node* declare_node = parse_declare();
				if (declare_node)
					objBlock->children.push_back(declare_node);
			}
		}

		if (currentToken().value != "private" && currentToken().value != "public" && currentToken().value != "protected" && tokens[cursor - 1].value == "{")
			throw runtime_error("Syntax error: 'access' keyword expected");
	}
	catch (const runtime_error& e) {
		return new Node("error", e.what());
	}

	return objBlock;
}

