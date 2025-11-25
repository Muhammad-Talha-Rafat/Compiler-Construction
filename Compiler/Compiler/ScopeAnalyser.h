#pragma once
#include <vector>
using namespace std;



class ScopeAnalyser {

private:

	Node* tree;
	Scope* root;
	Scope* currentScope;
	vector<ScopeError> errors;


	void analyse(Node* _tree);
	void enter(const string& _scope);
	void exit();
	void add(const Symbol& _symbol, Node* _node);
	Symbol* lookup(const string& _name, Scope* _scope);
	Scope* clone(const Scope* _root);
	void deleteScope(Scope* _scope);


public:

	ScopeAnalyser(Node* _tree);
	~ScopeAnalyser();

	Scope* getScopeTable();
	int warnings = 0;

};



ScopeAnalyser::ScopeAnalyser(Node* _tree) :tree(_tree) {
	root = new Scope();
	root->name = "GLOBAL";
	currentScope = root;
}



ScopeAnalyser::~ScopeAnalyser() {
	deleteScope(root);
}



Scope* ScopeAnalyser::getScopeTable() {
	analyse(tree->getNode("declarations"));
	return clone(root);
}



void ScopeAnalyser::analyse(Node* _tree) {

	if (!_tree) return;

	if (_tree->type == "define") {
		Symbol _define;
		_define.name = _tree->getValue("IDENTIFIER");

		for (Node* child : _tree->children) {
			if (child->type == "INTEGER" ||
				child->type == "DECIMAL" ||
				child->type == "CHARACTER" ||
				child->type == "STRLITERAL" ||
				child->type == "BOOLEAN") {
				_define.type = child->type;
				break;
			}
		}

		if (_define.type == "INTEGER") _define.type = "int";
		else if (_define.type == "DECIMAL") _define.type = "float";
		else if (_define.type == "CHARACTER") _define.type = "char";
		else if (_define.type == "STRLITERAL") _define.type = "string";
		else if (_define.type == "BOOLEAN") _define.type = "bool";

		_define.context = Context::variable;
		_define.isConst = true;
		add(_define, _tree);

		return;
	}

	if (_tree->type == "function") {

		Symbol _function;
		_function.name = _tree->getValue("IDENTIFIER");

		if (_function.name.empty())		// every function would have an IDENTIFIER child
			_function.name = "main";	// only case it won't have it will be when it is main

		_function.type = _tree->getValue("TYPE");
		_function.context = Context::function;

		add(_function, _tree);
		enter(_function.name);

		for (auto& child : _tree->children) {
			if (child->type == "parameter") {
				Symbol _parameter;
				_parameter.name = child->getValue("IDENTIFIER");
				_parameter.type = child->getValue("TYPE");
				_parameter.context = Context::parameter;
				add(_parameter, child);
			}
		}

		Node* _block = _tree->getNode("block");
		if (_block)	analyse(_block);

		exit();
		return;
	}


	if (_tree->type == "variable") {

		if (_tree->isThere("COMMA")) { // multi-variable declaration;

			auto current = _tree->children.begin();

			while (current != _tree->children.end()) {

				if ((*current)->type == "TYPE" || (*current)->type == "COMMA") {
					current++;
					continue;
				}

				if ((*current)->type == "SEMICOLON")
					break;

				Symbol _variable;
				_variable.name = (*current)->value;
				_variable.type = _tree->getValue("TYPE");
				_variable.context = Context::variable;

				add(_variable, *current);

				for (auto& child : (*current)->children)
					analyse(child);

				current++;
			}
		}

		else { // single variable declaration
			
			Symbol _variable;
			_variable.name = _tree->getValue("IDENTIFIER");
			_variable.type = _tree->getValue("TYPE");
			if (_tree->children[0]->value == "const")
				_variable.isConst = true;
			_variable.context = Context::variable;
			add(_variable, _tree);

			for (auto& child : _tree->children)
				if (child->type != "TYPE")
					analyse(child);
		}

		return;
	}

	if (_tree->type == "parameter") {
		Symbol _parameter;
		_parameter.name = _tree->getValue("IDENTIFIER");
		_parameter.type = _tree->getValue("TYPE");
		_parameter.context = Context::parameter;
		add(_parameter, _tree);

		return;
	}

	if (_tree->type == "argument") {
		Symbol _argument;
		_argument.name = _tree->getValue("IDENTIFIER");
		_argument.type = _tree->getValue("TYPE");
		_argument.context = Context::argument;
		add(_argument, _tree);

		return;
	}

	if (_tree->type == "for_loop") {
		enter("for");
		for (Node* child : _tree->children)
			analyse(child);
		exit();
		return;
	}

	if (_tree->type == "while_loop") {
		enter("while");
		for (Node* child : _tree->children)
			analyse(child);
		exit();
		return;
	}

	if (_tree->type == "do_while_loop") {
		enter("do while");

		for (Node* child : _tree->children)
			if (child->type == "block")
				analyse(child);
		exit();

		for (Node* child : _tree->children)
			if (child->type == "conditions")
				analyse(child);
		exit();

		return;
	}

	if (_tree->type == "if_block") {
		enter("if");
		for (Node* child : _tree->children)
			analyse(child);
		exit();
		return;
	}

	if (_tree->type == "else_if_block") {
		enter("else if");
		for (Node* child : _tree->children)
			analyse(child);
		exit();
		return;
	}

	if (_tree->type == "else_block") {
		enter("else");
		for (Node* child : _tree->children)
			analyse(child);
		exit();
		return;
	}

	if (_tree->type == "call_statement" || _tree->type == "function_call") {

		string _function = _tree->getValue("IDENTIFIER");

		Symbol* found = lookup(_function, currentScope);

		if (!found) {

			bool reported = false;

			for (const auto& symbol : currentScope->symbols) {
				if (symbol.context == Context::error && symbol.name.find(_function) != string::npos) {
					reported = true;
					break;
				}
			}

			if (!reported) {
				Symbol error;
				error.context = Context::error;
				error.error = ScopeError::UndefinedFunction;
				error.name = "\033[1;33mundefined function\033[0m \"" + string("\033[1;37m") + _function + "\033[0m\" in [" + "\033[0;90m" + currentScope->name + "\033[0m]";
				currentScope->symbols.push_back(error);
				warnings++;
			}
		}
		else if (found->context != Context::function) {

			bool reported = false;

			for (const auto& symbol : currentScope->symbols) {
				if (symbol.context == Context::error &&
					symbol.name.find(_function) != string::npos) {
					reported = true;
					break;
				}
			}

			if (!reported) {
				Symbol error;
				error.context = Context::error;
				error.error = ScopeError::UndefinedFunction;
				error.name = "\033[1;33m'" + _function + "' is not a function\033[0m in [" + "\033[0;90m" + currentScope->name + "\033[0m]";
				currentScope->symbols.push_back(error);
				warnings++;
			}
		}

		for (auto& child : _tree->children)
			if (child->type == "argument")
				analyse(child->children[0]);

		return;
	}

	if (_tree->type == "IDENTIFIER") {

		Symbol* found = lookup(_tree->value, currentScope);

		if (!found || found->context == Context::function) {

			bool reported = false;

			for (const auto& symbol : currentScope->symbols) {
				if (symbol.context == Context::error && symbol.name.find(_tree->value) != string::npos) {
					reported = true;
					break;
				}
			}

			if (!reported) {
				Symbol error;
				error.context = Context::error;
				error.error = ScopeError::UndeclaredVariable;
				if (!found) error.name = "\033[1;33mundeclared variable\033[0m \"" + string("\033[1;37m") + _tree->value + "\033[0m\" in [" + "\033[0;90m" + currentScope->name + "\033[0m]";
				else error.name = "(\033[1;33mvoid assignment\033[0m) function \"\033[1;37m" + _tree->value + "\033[0m\" cannot be used as a value in [" + "\033[0;90m" + currentScope->name + "\033[0m]";
				currentScope->symbols.push_back(error);
				warnings++;
			}
		}
		return;
	}

	for (auto& child : _tree->children)
		analyse(child);

}



void ScopeAnalyser::enter(const string& _scope) {
	Scope* newScope = new Scope();
	newScope->name = _scope;
	newScope->parent = currentScope;

	currentScope->children.push_back(newScope);
	currentScope = newScope;
}



void ScopeAnalyser::exit() {
	if (currentScope->parent) {
		currentScope = currentScope->parent;
	}
}





void ScopeAnalyser::add(const Symbol& _symbol, Node* _node) {

	for (const auto& symbol : currentScope->symbols) {
		if (symbol.name == _symbol.name) {
			Symbol error;
			error.context = Context::error;
			error.error = ScopeError::VariableRedefinition;
			error.name = "\033[1;33mredeclaration\033[0m of \"\033[1;37m" + _symbol.name + "\033[0m\" in [\033[0;90m" + currentScope->name + "\033[0m]";
			currentScope->symbols.push_back(error);
			warnings++;
			return;
		}
	}
	currentScope->symbols.push_back(_symbol);
}



Symbol* ScopeAnalyser::lookup(const string& _name, Scope* _scope) {

	if (!_scope) _scope = currentScope;

	for (auto& symbol : _scope->symbols)
		if (symbol.name == _name)
			return &symbol;

	if (_scope->parent)
		return lookup(_name, _scope->parent);

	return nullptr;
}



Scope* ScopeAnalyser::clone(const Scope* _root) {

	if (!_root) return nullptr;

	Scope* copy = new Scope();
	copy->name = _root->name;
	copy->parent = nullptr;
	copy->symbols = _root->symbols;

	for (const auto& child : _root->children) {
		Scope* childCopy = clone(child);
		childCopy->parent = copy;
		copy->children.push_back(childCopy);
	}

	return copy;
}



void ScopeAnalyser::deleteScope(Scope* _scope) {
	if (!_scope) return;
	for (auto child : _scope->children)
		deleteScope(child);
	delete _scope;
}




bool hasScopeWarnings(const ScopeAnalyser& analyser) {
	return analyser.warnings;
}



/*


[GLOBAL]
 number : variable (int)
 sum : function (void)
 main : function (int)

	[sum]
	 a : parameter (int)
	 b : parameter (int)
	 c : variable (int)

	[main]
	 a : variable (int)
	 b : variable (int)

		[for]
		 i : variable (int)
		 d : variable (int)

*/