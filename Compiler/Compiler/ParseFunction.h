#pragma once
using namespace std;



Node* Parser::parse_function(const string& type) {

	Node* function_node = new Node("function");

	function_node->children.push_back(new Node(expect("lBRACE")));

	if (currentToken().type != "rBRACE") { // means, there ARE arguments
		Node* arguments_node = parse_arguments();
		if (arguments_node)
			function_node->children.push_back(arguments_node);
	}

	function_node->children.push_back(new Node(expect("rBRACE")));
	function_node->children.push_back(new Node(expect("lPARENTHESIS")));

	if (currentToken().type != "rPARENTHESIS") {
		Node* statements_node = parse_statements();
		if (statements_node)
			function_node->children.push_back(statements_node);
	}

	if (currentToken().value != "return")
		throw runtime_error("Runtime error: expected a 'return' statement");
	
	Node* return_node = parse_return(type);
	if (return_node)
		function_node->children.push_back(return_node);

	function_node->children.push_back(new Node(expect("rPARENTHESIS")));

	return function_node;
}



Node* Parser::parse_voidfunction() {

	Node* voidfunction_node = new Node("void");

	voidfunction_node->children.push_back(new Node(expect("KEYWORD", "void")));

	if (tokens[cursor].value == "main") {
		voidfunction_node->children.push_back(new Node(expect(currentToken().type)));
		throw runtime_error("Runtime error: 'main' can only have return type 'int'");
	}

	voidfunction_node->children.push_back(new Node(expect("IDENTIFIER")));
	voidfunction_node->children.push_back(new Node(expect("lBRACE")));

	if (currentToken().type != "rBRACE") {
		Node* arguments_node = parse_arguments();
		if (arguments_node)
			voidfunction_node->children.push_back(arguments_node);
	}

	voidfunction_node->children.push_back(new Node(expect("rBRACE")));
	voidfunction_node->children.push_back(new Node(expect("lPARENTHESIS")));

	if (currentToken().type != "rPARENTHESIS") {
		Node* statements_node = parse_statements();
		if (statements_node)
			voidfunction_node->children.push_back(statements_node);
	}

	if (currentToken().value == "return") {
		voidfunction_node->children.push_back(new Node(expect(currentToken().type)));

		if (currentToken().type != "SEMICOLON")
			throw runtime_error("Runtime error: 'void' functions do not return anything");

		voidfunction_node->children.push_back(new Node(expect("SEMICOLON")));
	}

	voidfunction_node->children.push_back(new Node(expect("rPARENTHESIS")));

	return voidfunction_node;
}

Node* Parser::parse_mainfunction() {

	Node* mainfunction_node = new Node("main");

	mainfunction_node->children.push_back(new Node(expect(currentToken().type, "main")));

	if (tokens[cursor - 2].value != "int")
		throw runtime_error("Runtime error: 'main' can only have return type 'int'");
	
	mainfunction_node->children.push_back(new Node(expect("lBRACE")));

	if (currentToken().type != "rBRACE")
		throw runtime_error("Syntax error: 'main' could not contain arguments");
	
	mainfunction_node->children.push_back(new Node(expect("rBRACE")));
	mainfunction_node->children.push_back(new Node(expect("lPARENTHESIS")));

	Node* statements_node = parse_statements();
	if (statements_node);
		mainfunction_node->children.push_back(statements_node);
	
	if (currentToken() == token{ "KEYWORD", "return" }) {
		Node* return_node = parse_return("int");
		if (return_node)
			mainfunction_node->children.push_back(return_node);
	}
	
	mainfunction_node->children.push_back(new Node(expect("rPARENTHESIS")));

	return mainfunction_node;
}



Node* Parser::parse_arguments() {

	Node* arguments_node = new Node("arguments");

	Node* argument_node = parse_argument();
	if (argument_node)
		arguments_node->children.push_back(argument_node);

	while (currentToken().type == "COMMA") {

		arguments_node->children.push_back(new Node(expect(currentToken().type)));

		argument_node = parse_argument();
		if (argument_node)
			arguments_node->children.push_back(argument_node);
	}

	return arguments_node;
}



Node* Parser::parse_argument() {

	Node* argument_node = new Node("argument");

	if (currentToken().value == "const" || currentToken().value == "static")
		argument_node->children.push_back(new Node(expect(currentToken().type)));

	if (!types.count(currentToken().value))
		throw ExpectedTypeToken();

	argument_node->children.push_back(new Node(expect(currentToken().type)));
	argument_node->children.push_back(new Node(expect("IDENTIFIER")));

	return argument_node;
}



Node* Parser::parse_return(const string& type) {

	Node* return_node = new Node("return");

	return_node->children.push_back(new Node(expect("KEYWORD", "return")));

	if (currentToken().type != "SEMICOLON") {
		Node* expression_node = parse_expression(type);
		if (expression_node)
			return_node->children.push_back(expression_node);
	}

	return_node->children.push_back(new Node(expect("SEMICOLON")));

	return return_node;
}
