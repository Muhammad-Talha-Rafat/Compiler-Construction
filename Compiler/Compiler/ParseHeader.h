#pragma once
using namespace std;



Node* Parser::parse_headers() {

	if (peek().value != "#include" && peek().value != "using")
		return nullptr;

	Node* _headers = new Node("headers");

	while (peek().value == "#include" || peek().value == "using") {
		Node* header_node = parse_header();
		if (header_node)
			_headers->children.push_back(header_node);
		else break;
	}

	return _headers;
}



Node* Parser::parse_header() {

	Node* _header = new Node("header");

	try {

		if (peek().value == "#include") {

			_header->children.push_back(new Node(consume())); // #include

			if (peek().type == "LIBRARY" || peek().type == "HEADER")
				_header->children.push_back(new Node(consume())); // LIBRARY / HEADER
			else throw ParseError::UnexpectedToken("LIBRARY\" or \"HEADER", peek());

		}
		else if (peek().value == "using") {

			_header->children.push_back(new Node(expectValue("using")));
			_header->children.push_back(new Node(expectValue("namespace")));
			_header->children.push_back(new Node(expectType("IDENTIFIER")));
			_header->children.push_back(new Node(expectType("SEMICOLON")));

		}
		else _header = nullptr; // gonna return no header 

		return _header;
	}
	catch (const runtime_error& e) {
		return new Node("error", e.what());
	}
}
