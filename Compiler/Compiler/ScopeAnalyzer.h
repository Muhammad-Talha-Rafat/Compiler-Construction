#pragma once
#include <vector>
#include <unordered_map>
#include <iomanip>
using namespace std;



class ScopeAnalyzer {

private:

	enum Context { variable, function, argument, parameter, error };

	struct Symbol {
		string name;
		string type;
		Context context;
	};

	struct Scope {
		string name;
		Scope* parent = nullptr;
		vector<Symbol> symbols;
		vector<Scope*> children;
	};

	void deleteScope(Scope* scope) {
		for (auto child : scope->children)
			deleteScope(child);
		delete scope;
	}

	Scope* root;
	Scope* currentScope;

public:

	ScopeAnalyzer() {
		root = new Scope();
		root->name = "GLOBAL";
		root->parent = nullptr;
		currentScope = root;
	}

	void analyse(Node* tree);
	void enter(const string& _scope);
	void exit();
	void add(const Symbol& _symbol);
	Symbol* lookup(const string& _name, Scope* _scope);
	void print();
	void printScope(Scope* scope, int indent);

	~ScopeAnalyzer() {
		deleteScope(root);
	}

};



void ScopeAnalyzer::analyse(Node* tree) {

	if (!tree) return;

	if (tree->type == "function") {

		Symbol _function;
		_function.name = tree->getValue("IDENTIFIER");

		if (_function.name.empty())		// every function would have an IDENTIFIER child
			_function.name = "main";	// only case it won't have it will be when it is main

		_function.type = tree->getValue("TYPE");
		_function.context = Context::function;
		add(_function);
		enter(_function.name);

		for (auto& child : tree->children) {
			if (child->type == "parameter") {
				Symbol _parameter;
				_parameter.name = child->getValue("IDENTIFIER");
				_parameter.type = child->getValue("TYPE");
				_parameter.context = Context::parameter;
				add(_parameter);
			}
		}

		Node* _block = tree->getNode("block");
		if (_block)	analyse(_block);

		exit();
		return;
	}


	if (tree->type == "variable") {
		Symbol _variable;
		_variable.name = tree->getValue("IDENTIFIER");
		_variable.type = tree->getValue("TYPE");
		_variable.context = Context::variable;
		add(_variable);
		return;
	}

	if (tree->type == "parameter") {
		Symbol _parameter;
		_parameter.name = tree->getValue("IDENTIFIER");
		_parameter.type = tree->getValue("TYPE");
		_parameter.context = Context::parameter;
		add(_parameter);
		return;
	}

	if (tree->type == "argument") {
		Symbol _argument;
		_argument.name = tree->getValue("IDENTIFIER");
		_argument.type = tree->getValue("TYPE");
		_argument.context = Context::argument;
		add(_argument);
		return;
	}

	if (tree->type == "for_loop") {
		enter("for");
		for (Node* child : tree->children)
			if (child->value == "")
				analyse(child);
		exit();
		return;
	}

	if (tree->type == "while_loop") {
		enter("while");
		for (Node* child : tree->children)
			if (child->value == "")
				analyse(child);
		exit();
		return;
	}

	if (tree->type == "do_while_loop") {
		enter("do while");
		for (Node* child : tree->children)
			if (child->value == "")
				analyse(child);
		exit();
		return;
	}

	if (tree->type == "if_block") {
		enter("if");
		for (Node* child : tree->children)
			if (child->value == "")
				analyse(child);
		exit();
		return;
	}

	if (tree->type == "else_if_block") {
		enter("else if");
		for (Node* child : tree->children)
			if (child->value == "")
				analyse(child);
		exit();
		return;
	}

	if (tree->type == "else_block") {
		enter("else");
		for (Node* child : tree->children)
			if (child->value == "")
				analyse(child);
		exit();
		return;
	}

	for (auto& child : tree->children) {
		if (child->value == "")
			analyse(child);
	}
}





void ScopeAnalyzer::enter(const string& _scope) {
	Scope* newScope = new Scope();
	newScope->name = _scope;
	newScope->parent = currentScope;

	currentScope->children.push_back(newScope);
	currentScope = newScope;
}





void ScopeAnalyzer::exit() {
	if (currentScope->parent) {
		currentScope = currentScope->parent;
	}
}





void ScopeAnalyzer::add(const Symbol& _symbol) {
	for (const auto& sym : currentScope->symbols) {
		if (sym.name == _symbol.name) {
			Symbol error;
			error.context = Context::error;
			error.type = "";
			error.name = "redeclaration of \"\033[0;32m" + _symbol.name + "\033[0m\" in [\033[0;90m" + currentScope->name + "\033[0m]";
			currentScope->symbols.push_back(error);
			return;
		}
	}

	// Add symbol to current scope
	currentScope->symbols.push_back(_symbol);
}



ScopeAnalyzer::Symbol* ScopeAnalyzer::lookup(const string& _name, Scope* _scope) {
	// Default to current scope if not provided
	if (!_scope) _scope = currentScope;

	// Search in this scope
	for (auto& sym : _scope->symbols) {
		if (sym.name == _name) {
			return &sym;  // found
		}
	}

	// Recurse to parent if not found
	if (_scope->parent) {
		return lookup(_name, _scope->parent);
	}

	return nullptr;  // symbol not found
}



void ScopeAnalyzer::print() {
	printScope(root, 0);
}

void ScopeAnalyzer::printScope(Scope* _scope, int indent) {
	if (!_scope) return;

	string spacing(indent * 2, ' ');
	cout << spacing << "[\033[0;90m" << _scope->name << "\033[0m]\n";

	for (const auto& symbol : _scope->symbols) {
		if (symbol.context == Context::error) {
			cout << spacing << "  \033[1;31mWarning\033[0m: " << symbol.name << endl;
		}
		else {
			cout << spacing << "  " << symbol.name << " : \033[0;36m";
			switch (symbol.context) {
			case Context::variable: cout << "variable "; break;
			case Context::function: cout << "function "; break;
			case Context::argument: cout << "argument "; break;
			case Context::parameter: cout << "parameter"; break;
			}
			cout << "\033[0m" << " (\033[0;91m" << symbol.type << "\033[0m)\n";
		}
	}
	cout << endl;

	for (auto child : _scope->children) {
		printScope(child, indent + 1);
	}
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