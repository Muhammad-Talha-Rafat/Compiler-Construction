#pragma once
#include <unordered_map>
using namespace TypeErrors;



class TypeChecker
{
private:

    Node* root;
    Scope* scopeTable;

    unordered_map<Scope*, unordered_map<string, int>> scope_counter;

    string lookup_type(const string& _identifier);
    bool isLiteral(const string& _type);
    string getLiteralType(const string& _literal);

    string get_operation_type(Node* _operation);
    string get_expression_type(Node* _node);
    bool validate_arguments(Node* _function_call);

    void check_node(Node* _node);

public:

    TypeChecker(Node* _root, Scope* _scopeTable)
        :root(_root), scopeTable(_scopeTable) {
    }

    void check();
    int warnings = 0;

};





bool TypeChecker::isLiteral(const string& _type) {
    return (_type == "INTEGER" ||
        _type == "DECIMAL" ||
        _type == "CHARACTER" ||
        _type == "STRLITERAL" ||
        _type == "BOOLEAN");
}





string TypeChecker::getLiteralType(const string& _literal) {
    if (_literal == "INTEGER") return "int";
    if (_literal == "DECIMAL") return "float";
    if (_literal == "CHARACTER") return "char";
    if (_literal == "STRLITERAL") return "string";
    if (_literal == "BOOLEAN") return "bool";
    return "unknown";
}





string TypeChecker::lookup_type(const string& _identifier) {

    Scope* currentScope = scopeTable;

    while (currentScope) {

        for (const Symbol& symbol : currentScope->symbols) {
            if (symbol.name == _identifier) {
                if (symbol.context == Context::variable || symbol.context == Context::parameter || symbol.context == Context::function) {
                    return symbol.type;
                }
            }
        }

        currentScope = currentScope->parent;
    }

    return "";
}





void TypeChecker::check() {

    if (!root) return;

    Node* _declarations = root->getNode("declarations");
    if (!_declarations) return;

    for (auto _node : _declarations->children)
        check_node(_node);
}





void TypeChecker::check_node(Node* _node) {

    if (_node->type == "variable" || _node->type == "assignment") {
        // it's a variable declaration/assignment

        string variable_name = _node->getValue("IDENTIFIER");
        string variable_type;

        if (_node->type == "variable") {
            variable_type = _node->getValue("TYPE");
        }
        else {
            variable_type = lookup_type(variable_name);
            if (variable_type.empty()) {
                _node->children.push_back(new Node("error", TypeMismatch(token{ variable_type, variable_name }, "variable_type").what()));
                warnings++;
                return;
            }
        }
        
        Node* assigned_value = nullptr;
        bool _assign = false;

        for (Node* child : _node->children) {
            if (child->type == "SEMICOLON") break;

            if (_assign) {
                assigned_value = child;
                break;
            }

            if (child->type == "ASSIGN") _assign = true;
        }

        Scope* current = scopeTable;
        Symbol* symbol = nullptr;
        while (current && !symbol) {
            for (auto& _symbol : current->symbols) {
                if (_symbol.name == variable_name) {
                    symbol = &_symbol;
                    break;
                }
            }
            current = current->parent;
        }

        if (_node->type == "assignment" && symbol->isConst) {
            _node->children.push_back(new Node("error", ConstAssignment(variable_name).what()));
            warnings++;
            return;
        }


        if (assigned_value) {

            string assigned_type = get_expression_type(assigned_value);

            if (variable_type != assigned_type && assigned_type != "NA" && assigned_type != "void") {
                _node->children.push_back(new Node("error", TypeMismatch(token{ variable_type, variable_name }, assigned_type).what()));
                warnings++;
            }
        }
        return;
    }

    if (_node->type == "function") {
        // it's a return-type function
        string function_type = _node->getValue("TYPE");

        Scope* previous = scopeTable;
        if (_node->getValue("KEYWORD") == "main")
            scopeTable = scopeTable->getScope("main");
        else scopeTable = scopeTable->getScope(_node->getValue("IDENTIFIER"));

        Node* block = _node->getNode("block");
        if (block) {
            for (auto statement : block->children) {
                check_node(statement);
                if (statement->type == "return") {
                    Node* return_value = statement->children[1];
                    string return_type = get_expression_type(return_value);
                    if (function_type != return_type) {
                        statement->children.push_back(new Node("error", ReturnMismatch(token{ function_type, _node->getValue("IDENTIFIER") }, return_type).what()));
                        warnings++;
                    }
                }
            }
        }
        scopeTable = previous;
        return;
    }

    if (_node->type == "increment" || _node->type == "decrement") {
        get_expression_type(_node);
        return;
    }

    if (_node->type == "call_statement") {
        lookup_type(_node->getValue("IDENTIFIER"));
        validate_arguments(_node);
        return;
    }

    if (_node->type == "output") {
        for (auto child : _node->children) {
            if (child->type == "IDENTIFIER" || child->type == "operation" || child->type == "expression" || child->type == "function_call" || child->type == "increment" || child->type == "decrement") {
                get_expression_type(child);
            }
        }
        return;
    }

    if (_node->type == "comparison") {
        get_operation_type(_node);
        return;
    }

    if (_node->type == "if_block" || _node->type == "else_if_block" || _node->type == "else_block" || _node->type == "for_loop" || _node->type == "while_loop" || _node->type == "do_while_loop") {

        string scope_name;

        if (_node->type == "if_block") scope_name = "if";
        else if (_node->type == "else_if_block") scope_name = "else if";
        else if (_node->type == "else_block") scope_name = "else";
        else if (_node->type == "for_loop") scope_name = "for";
        else if (_node->type == "while_loop") scope_name = "while";
        else if (_node->type == "do_while_loop") scope_name = "do while";

        Scope* parent = scopeTable;
        int instance = scope_counter[parent][scope_name]++;
        scopeTable = parent->getScope(scope_name, instance);

        Node* conditions = _node->getNode("conditions");
        if (conditions) {
            for (auto condition : conditions->children) {
                if (condition->type == "comparison") {
                    string condition_type = get_operation_type(condition);
                }
            }
        }

        Node* block = _node->getNode("block");
        if (block) {
            for (auto statement : block->children) {
                check_node(statement);
            }
        }

        scopeTable = parent;
        return;
    }


    for (auto* child : _node->children)
        check_node(child);

}





string TypeChecker::get_operation_type(Node* _operation) {

    if (_operation->type == "expression") {
        if (_operation->children.size() != 3) return "NA";
        return get_operation_type(_operation->children[1]);
    }

    if (_operation->children.size() != 3) return "NA";

    Node* left = _operation->children[0];
    // children[1] is the operator, for sure
    Node* right = _operation->children[2];

    string left_type = get_expression_type(left);
    string right_type = get_expression_type(right);

    if (left_type != right_type && left_type != "NA" && right_type != "NA") {
        _operation->children.push_back(new Node("error", ExpressionType(left_type, _operation->children[1]->value, right_type).what()));
        warnings++;
        return "NA";
    }

    return left_type;
}





string TypeChecker::get_expression_type(Node* _node) {

    string _type;

    if (isLiteral(_node->type))
        _type = getLiteralType(_node->type);

    else if (_node->type == "IDENTIFIER")
        _type = lookup_type(_node->value);

    else if (_node->type == "function_call") {
        _type = lookup_type(_node->getValue("IDENTIFIER"));

        if (_type == "void") { // are you trying to assign  avoid function?
            _node->children.push_back(new Node("error", AssigningVoid(_node->getValue("IDENTIFIER")).what()));
            warnings++;
        }

        validate_arguments(_node);
    }

    else if (_node->type == "increment" || _node->type == "decrement") {
        _type = lookup_type(_node->getValue("IDENTIFIER"));

        if (_type != "int" && _type != "float") {
            _node->children.push_back(new Node("error", SyntaxError("increment/decrement requires <\033[1;33mint\033[0m> or <\033[1;33mfloat\033[0m> type but <\033[1;33m" + _type + "\033[0m> encountered").what()));
            warnings++;
        }
    }

    else if (_node->type == "operation" || _node->type == "expression")
        _type = get_operation_type(_node);

    else _type = "NA";

    return _type;
}





bool TypeChecker::validate_arguments(Node* _function_call) {

    Scope* root = scopeTable;
    while (root->parent) root = root->parent;

    Scope* _function = root->getScope(_function_call->getValue("IDENTIFIER"));

    // collect all argument nodes
    vector<Node*> arguments;
    for (auto child : _function_call->children) {
        if (child->type == "argument") {
            // since argument node contains only one object and we know it's children[0]
            arguments.push_back(child->children[0]);
        }
    }

    // collect all parameter nodes
    vector<Symbol> parameters;
    for (auto symbol : _function->symbols) {
        if (symbol.context == Context::parameter)
            parameters.push_back(symbol);
    }

    // match the number of arguments and parameters
    if (arguments.size() != parameters.size()) {
        _function_call->children.push_back(new Node("error", ParameterMiscount(_function_call->getValue("IDENTIFIER")).what()));
        warnings++;
        return false;
    }

    // check types iteratively

    for (size_t i = 0; i < arguments.size(); i++) {

        string argument_type = get_expression_type(arguments[i]);

        if (argument_type == "NA") {
            _function_call->children.push_back(new Node("error", SyntaxError("unknown argument type at position \033[1;33m" + to_string(i + 1) + string("\033[0m")).what()));
            warnings++;
            return false;
        }

        if (argument_type != parameters[i].type) {
            _function_call->children.push_back(new Node("error", ParameterTypeMismatch(_function_call->getValue("IDENTIFIER"), i + 1).what()));
            warnings++;
            return false;
        }
    }

    return true;
}





// outlaw
bool hasTypeWarnings(TypeChecker& checker) {
    if (checker.warnings != 0) {
        cout << "[\033[1;31mCritical\033[0m] Unfortunately you have encountered \033[1;33m" << checker.warnings << "\033[0m type warning" << (checker.warnings == 1 ? "" : "s")
            << endl << "Resolve them to continue to IR generation";
        return true;
    }
    return false;
}