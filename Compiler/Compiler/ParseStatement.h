#pragma once
using namespace std;



Node* Parser::parse_statements() {
	set<string> statementset = { "++", "--", "for", "while", "do", "else if", "if", "else", "cout", "cin", "const", "static" };
	
	Node* statements = new Node("statements");

	while (currentToken().type == "IDENTIFIER" || statementset.count(currentToken().value) || types.count(currentToken().value)) {
		Node* statement_node = parse_statement();
		if (statement_node)
			statements->children.push_back(statement_node);
	}

	return statements;
}



Node* Parser::parse_statement() {

	Node* statement = new Node("statement");

	if (currentToken().type == "IDENTIFIER") {

		token identifier = expect("IDENTIFIER");
		Node* identifier_node = new Node(identifier);

		if (assignment_op.count(currentToken().type)) { // assignment

			Node* assign_node = new Node("assignment");

			assign_node->children.push_back(identifier_node);
			assign_node->children.push_back(new Node(expect(currentToken().type)));

			Node* expression_node = parse_expression();
			if (!expression_node)
				throw runtime_error("Expected an expression");
			else assign_node->children.push_back(expression_node);

			assign_node->children.push_back(new Node(expect("SEMICOLON")));

			statement->children.push_back(assign_node);
		}
		else if (currentToken().type == "lBRACE") { // call statement

			Node* function_call_node = new Node("functionCall");

			function_call_node->children.push_back(identifier_node);
			function_call_node->children.push_back(new Node(expect("lBRACE")));

			if (currentToken().type == "IDENTIFIER"
				|| literals.count(currentToken().type)
				|| tokens[cursor].value == "true"
				|| tokens[cursor].value == "false") {

				Node* parameters_node = parse_parameters();
				if (parameters_node)
					function_call_node->children.push_back(parameters_node);
			}

			function_call_node->children.push_back(new Node(expect("rBRACE")));
			function_call_node->children.push_back(new Node(expect("SEMICOLON")));

			statement->children.push_back(function_call_node);
		}
		else if (currentToken().type == "INCREMENT" || currentToken().type == "DECREMENT") { // post increment/decrement
			Node* IncDec_node = new Node(currentToken().value == "++" ? "increment" : "decrement");

			IncDec_node->children.push_back(identifier_node);
			IncDec_node->children.push_back(new Node(expect(currentToken().type)));
			IncDec_node->children.push_back(new Node(expect("SEMICOLON")));

			statement->children.push_back(IncDec_node);
		}
		else throw runtime_error("Syntax error: expected a valid statement");

	}
	else if (currentToken().value == "const" || currentToken().value == "static" || types.count(currentToken().value)) {

		Node* declare_node = parse_declare(); // declaration / initialization
		if (declare_node)
			statement->children.push_back(declare_node);

	}
	else if (currentToken().value == "++" || currentToken().value == "--") { // pre increment/decrement

		Node* IncDec_node = new Node(currentToken().value == "++" ? "increment" : "decrement");

		IncDec_node->children.push_back(new Node(expect(currentToken().type)));
		IncDec_node->children.push_back(new Node(expect("IDENTIFIER")));
		IncDec_node->children.push_back(new Node(expect("SEMICOLON")));

		statement->children.push_back(IncDec_node);

	}
	else if (currentToken().value == "cout" || currentToken().value == "cin") { // I/O

		Node* iostream_node = new Node(currentToken().value == "cout" ? "output" : "input");

		Node* stream_node = parse_iostream(currentToken().value);
		if (stream_node)
			iostream_node->children.push_back(stream_node);

		statement->children.push_back(iostream_node);

	}
	else if (currentToken().value == "if") { // if block

		Node* if_node = new Node("if_block");

		if_node->children.push_back(new Node(expect(currentToken().type)));
		if_node->children.push_back(new Node(expect("lBRACE")));

		if (currentToken().type != "rBRACE") {
			Node* conditions_node = parse_conditions();
			if (conditions_node)
				if_node->children.push_back(conditions_node);
		}

		if_node->children.push_back(new Node(expect("rBRACE")));

		// {block}

		if_node->children.push_back(new Node(expect("lPARENTHESIS")));

		if (currentToken().type != "rPARENTHESIS") {
			Node* if_body_node = parse_statements();
			if (if_body_node)
				if_node->children.push_back(if_body_node);
		}

		if_node->children.push_back(new Node(expect("rPARENTHESIS")));

		while (currentToken().value == "else if") {

			Node* elseif_node = new Node("elseif_block");

			elseif_node->children.push_back(new Node(expect(currentToken().type)));
			elseif_node->children.push_back(new Node(expect("lBRACE")));

			if (currentToken().type != "rBRACE") {
				Node* conditions_node = parse_conditions();
				if (conditions_node)
					elseif_node->children.push_back(conditions_node);
			}

			elseif_node->children.push_back(new Node(expect("rBRACE")));

			// {block}

			elseif_node->children.push_back(new Node(expect("lPARENTHESIS")));

			if (currentToken().type != "rPARENTHESIS") {
				Node* elseif_body_node = parse_statements();
				if (elseif_body_node)
					elseif_node->children.push_back(elseif_body_node);
			}

			elseif_node->children.push_back(new Node(expect("rPARENTHESIS")));
			if_node->children.push_back(elseif_node);
		}

		if (currentToken().value == "else") {

			Node* else_node = new Node("else_block");

			else_node->children.push_back(new Node(expect(currentToken().type)));

			// no condition

			else_node->children.push_back(new Node(expect("lPARENTHESIS")));

			if (currentToken().type != "rPARENTHESIS") {
				Node* else_body_node = parse_statements();
				if (else_body_node)
					else_node->children.push_back(else_body_node);
			}

			else_node->children.push_back(new Node(expect("rPARENTHESIS")));

			if_node->children.push_back(else_node);
		}

		statement->children.push_back(if_node);

	}
	else if (currentToken().value == "else if" || currentToken().value == "else") {

		statement->children.push_back(new Node(expect(currentToken().type)));
		throw runtime_error("Syntax error: expected an 'if' statement before this");

	}
	else if (currentToken().value == "for") { // 

		Node* for_loop = new Node("for_loop");

		for_loop->children.push_back(new Node(expect(currentToken().type)));
		for_loop->children.push_back(new Node(expect("lBRACE")));

		// assignment

		string type = "int"; // default
		Node* assign_node = new Node("init");
		if (types.count(currentToken().value)) {
			type = currentToken().value;
			assign_node->children.push_back(new Node(expect(currentToken().type)));
		}

		assign_node->children.push_back(new Node(expect("IDENTIFIER")));
		assign_node->children.push_back(new Node(expect("ASSIGN")));

		if (currentToken().type == "SEMICOLON")
			throw ExpectedExpression(type);

		Node* expression_node = parse_expression(type);
		if (expression_node)
			assign_node->children.push_back(expression_node);

		for_loop->children.push_back(assign_node);

		for_loop->children.push_back(new Node(expect("SEMICOLON")));

		// condition

		Node* condition_node = new Node("condition");

		if (currentToken().type == "SEMICOLON")
			throw UnexpectedToken("condition", currentToken());

		Node* conditions_node = parse_conditions();
		if (conditions_node)
			for_loop->children.push_back(conditions_node);

		for_loop->children.push_back(new Node(expect("SEMICOLON")));

		// update

		Node* update_node = new Node("update");

		if (currentToken().type == "INCREMENT" || currentToken().type == "DECREMENT") {

			update_node->children.push_back(new Node(expect(currentToken().type)));
			update_node->children.push_back(new Node(expect("IDENTIFIER")));

		}
		else if (currentToken().type == "IDENTIFIER") {

			update_node->children.push_back(new Node(expect(currentToken().type)));

			if (currentToken().type == "INCREMENT" || currentToken().type == "DECREMENT")
				update_node->children.push_back(new Node(expect(currentToken().type)));

		}
		else throw runtime_error("Syntax error: give a valid update statement");

		for_loop->children.push_back(update_node);

		for_loop->children.push_back(new Node(expect("rBRACE")));

		// {block}

		for_loop->children.push_back(new Node(expect("lPARENTHESIS")));

		if (currentToken().type != "rPARENTHESIS") {
			Node* statements_node = parse_statements();
			if (statements_node)
				for_loop->children.push_back(statements_node);
		}

		for_loop->children.push_back(new Node(expect("rPARENTHESIS")));

		statement->children.push_back(for_loop);

	}
	else if (currentToken().value == "while") {

		Node* while_loop = new Node("while_loop");

		while_loop->children.push_back(new Node(expect(currentToken().type)));
		while_loop->children.push_back(new Node(expect("lBRACE")));

		if (currentToken().type != "rBRACE") {
			Node* conditions_node = parse_conditions();
			if (conditions_node)
				while_loop->children.push_back(conditions_node);
		}

		while_loop->children.push_back(new Node(expect("rBRACE")));

		// {block}
		
		while_loop->children.push_back(new Node(expect("lPARENTHESIS")));
		
		if (currentToken().type != "rPARENTHESIS") {
			Node* statements_node = parse_statements();
			if (statements_node)
				while_loop->children.push_back(statements_node);
		}

		while_loop->children.push_back(new Node(expect("rPARENTHESIS")));

		statement->children.push_back(while_loop);

	}
	else if (currentToken().value == "do") {

		Node* dowhile_loop = new Node("dowhile_loop");

		dowhile_loop->children.push_back(new Node(expect(currentToken().type)));

		// {block}

		dowhile_loop->children.push_back(new Node(expect("lPARENTHESIS")));

		if (currentToken().type != "rPARENTHESIS") {
			Node* statements_node = parse_statements();
			if (statements_node)
				dowhile_loop->children.push_back(statements_node);
		}

		dowhile_loop->children.push_back(new Node(expect("rPARENTHESIS")));

		// (conditions)

		dowhile_loop->children.push_back(new Node(expect("KEYWORD", "while")));
		dowhile_loop->children.push_back(new Node(expect("lBRACE")));

		if (currentToken().type != "rBRACE") {
			Node* conditions_node = parse_conditions();
			if (conditions_node)
				dowhile_loop->children.push_back(conditions_node);
		}

		dowhile_loop->children.push_back(new Node(expect("rBRACE")));
		dowhile_loop->children.push_back(new Node(expect("SEMICOLON")));

		statement->children.push_back(dowhile_loop);

	}

	return statement;
}



Node* Parser::parse_iostream(const string& stream) {

	Node* iostream = new Node("iostream");

	if (stream == "cout") {
		iostream->children.push_back(new Node(expect("KEYWORD", "cout")));

		Node* ostring_node = parse_ostring();
		if (ostring_node)
			iostream->children.push_back(ostring_node);

		iostream->children.push_back(new Node(expect("SEMICOLON")));
	}
	else { // stream == "cin"

		iostream->children.push_back(new Node(expect("KEYWORD", "cin")));
		iostream->children.push_back(new Node(expect("RIGHT_SHIFT")));
		iostream->children.push_back(new Node(expect("IDENTIFIER")));
		iostream->children.push_back(new Node(expect("SEMICOLON")));
	}

	return iostream;
}



Node* Parser::parse_ostring() {

	Node* ostring = new Node("ostring");

	ostring->children.push_back(new Node(expect("LEFT_SHIFT")));

	if (currentToken() == token{ "KEYWORD", "endl" })
		ostring->children.push_back(new Node(expect(currentToken().type)));
	else {
		Node* expression_node = parse_expression();
		if (expression_node)
			ostring->children.push_back(expression_node);
	}

	if (currentToken().type == "LEFT_SHIFT") {
		Node* ostring_node = parse_ostring();
		if (ostring_node)
			ostring->children.push_back(ostring_node);
	}

	return ostring;
}



Node* Parser::parse_conditions() {

	Node* conditions = new Node("conditions");

	Node* condition_node = parse_condition();
	if (condition_node)
		conditions->children.push_back(condition_node);

	if (logical_op.count(currentToken().type)) {
		conditions->children.push_back(new Node(expect(currentToken().type)));

		Node* conditions_node = parse_conditions();
		if (conditions_node)
			conditions->children.push_back(conditions_node);
	}

	return conditions;
}



Node* Parser::parse_condition() {

	Node* condition = new Node("condition");

	if (currentToken().type == "lBRACE") {
		condition->children.push_back(new Node(expect(currentToken().type)));

		Node* conditions_node = parse_conditions();
		if (conditions_node)
			condition->children.push_back(conditions_node);

		condition->children.push_back(new Node(expect("rBRACE")));
	}
	else {
		Node* comparison_node = parse_comparison();
		if (comparison_node)
			condition->children.push_back(comparison_node);
	}

	return condition;
}



Node* Parser::parse_comparison() {
	Node* comparison = new Node("comparison");

	Node* expression_node = parse_expression();
	if (expression_node)
		comparison->children.push_back(expression_node);

	if (!comparison_op.count(currentToken().type))
		throw UnexpectedToken("comparison operator", currentToken());
	else comparison->children.push_back(new Node(expect(currentToken().type)));

	expression_node = parse_expression();
	if (expression_node)
		comparison->children.push_back(expression_node);

	return comparison;
}
