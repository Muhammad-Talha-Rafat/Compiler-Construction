#pragma once
#include <string>
#include <vector>
using namespace std;



namespace ParseError {

    runtime_error UnexpectedEOF() {
        return runtime_error("\033[1;31mUnexpectedEOF\033[0m: end of code was not expected");
    }

    runtime_error UnexpectedToken(const string& expected, const token& encountered) {
        string value;
        if (expected == "value" || expected == "type" || expected == "declaration") value = expected;
        else value = "\"" + expected + "\"";
        if (encountered.type == "UNDEFINED") {
            string undefined = encountered.value == "\"" ? "\\\"" : encountered.value;
            return runtime_error("\033[1;31mUnexpectedToken\033[0m: expected a " + value + ", got \"" + undefined + "\"");
        }
        return runtime_error("\033[1;31mUnexpectedToken\033[0m: expected " + value + ", got \"" + encountered.type + "\"");
    }

    runtime_error ExpectedIdentifier(const string& encountered) {
        return runtime_error("\033[1;31mExpectedIdentifier\033[0m: expected an <IDENTIFIER> but \"" + encountered + "\" identified");
    }

    runtime_error UndefinedToken(const string& encountered) {
        if (encountered == "\"") return runtime_error("\033[1;31mUndefinedToken\033[0m: \"\\" + encountered + "\" found");
        else return runtime_error("\033[1;31mUndefinedToken\033[0m: \"" + encountered + "\" found");
    }

    runtime_error ExpectedTypeToken() {
        return runtime_error("\033[1;31mExpectedTypeToken\033[0m: unable to find data type");
    }

    runtime_error ExpectedExpression() {
        return runtime_error("\033[1;31mExpectedExpression\033[0m: an expression was expected here");
    }

    runtime_error MissingReturn() {
        return runtime_error("\033[1;31mMissingReturn\033[0m: Missing \"return\" statement in non-void function");
    }

    runtime_error SyntaxError(const string& message) {
        return runtime_error("\033[1;31mSyntax error\033[0m: " + message);
    }
}


namespace TypeErrors {

    runtime_error TypeMismatch(const token& variable, const string& encountered) {
        return runtime_error("\033[1;31mtype mismatch\033[0m: a variable (\033[1;33m" + variable.value + "\033[0m) of type <\033[1;33m" + variable.type + "\033[0m> cannot be assigned <\033[1;33m" + encountered +"\033[0m> value");
    }

    runtime_error ExpressionType(const string& left_type, const string& _operator, const string& right_type) {
        return runtime_error("\033[1;31mtype mismatch\033[0m: case <\033[1;33m" + left_type + "\033[0m> " + _operator + " <\033[1;33m" + right_type + "\033[0m>: types on either side of the operation do not match");
    }

    runtime_error AssigningVoid(const string& function_name) {
        return runtime_error("\033[1;31mtype error\033[0m: cannot assign a <\033[1;33mvoid\033[0m> function (\033[1;33m" + function_name + "\033[0m) to a variable");
    }

    runtime_error ParameterMiscount(const string& function_name) {
        return runtime_error("\033[1;31mparameter miscount\033[0m: number of arguments in function call (\033[1;33m" + function_name + "\033[0m) do not match");
    }

    runtime_error ParameterTypeMismatch(const string& function_name, const int& position) {
        return runtime_error("\033[1;31mtype mismatch\033[0m: function call (\033[1;33m" + function_name + "\033[0m) received a mismatching parameter type at position \033[1;33m" + to_string(position) + "\033[0m");
    }

    runtime_error ReturnMismatch(const token& function, const string& encountered) {
        return runtime_error("\033[1;31mreturn mismatch\033[0m: a function (\033[1;33m" + function.value + "\033[0m) with return type <\033[1;33m" + function.type + "\033[0m> cannot return <\033[1;33m" + encountered + "\033[0m> value");
    }
}



enum annotation {
    _const,
    _static,
    _int,
    _float,
    _double,
    _char,
    _string,
    _bool,
    _void,
    _error
};


struct Node {


    string type;
    string value;
    vector<Node*> children;
    vector<annotation> annotations;
    vector<string> errors;

    Node(const string& _type, const string& _value = "")
        : type(_type), value(_value) {}

    Node(const token& _token)
        : type(_token.type), value(_token.value) {}


    void print(int depth = 0) const {
        for (int i = 0; i < depth; i++)
            cout << "   ";

        if (type == "error") {
            cout << value << endl;
            return;
        }

        if (!children.empty())
            cout << "\033[1;37m" << type << " \033[0m";
        else cout << "\033[1;33m" << type << "\033[0m";

        if (!value.empty())
            cout << ": " << value;

        if (!annotations.empty()) {
            cout << "[";
            for (size_t i = 0; i < annotations.size(); ++i) {
                cout << "\033[0;90m";
                switch (annotations[i]) {
                case _const: cout << "const"; break;
                case _static: cout << "static"; break;
                case _int: cout << "int"; break;
                case _float: cout << "float"; break;
                case _double: cout << "double"; break;
                case _char: cout << "char"; break;
                case _string: cout << "string"; break;
                case _bool: cout << "bool"; break;
                case _void: cout << "void"; break;
                }
                cout << "\033[0m";
                if (i + 1 < annotations.size()) cout << ", ";
            }
            cout << "]";
        }


        cout << endl;

        for (Node* child : children)
            if (child)
                child->print(depth + 1);
    }


    string getValue(const string& _type) {
        for (auto* child : this->children)
            if (child->children.empty() && child->type == _type)
                return child->value;
        return "";
    }

    Node* getNode(const string& _type) {
        for (auto* child : this->children)
            if (!child->children.empty() && child->type == _type && child->value.empty())
                return child;
        return nullptr;
    }

    bool isThere(const string& _type) {
        for (auto* child : this->children)
            if (child->type == _type)
                return true;
        return false;
    }
 
    ~Node() {
        for (auto child : children)
            delete child;
    }

};



enum ScopeError {
    none,
    UndeclaredVariable,
    UndefinedFunction,
    VariableRedefinition,
    FunctionRedefinition
};

enum Context {
    variable,
    function,
    argument,
    parameter,
    error
};



struct Symbol {
    string name;
    string type;
    Context context;
    ScopeError error;
    vector<annotation> annotations;
};



struct Scope {

    string name;
    Scope* parent = nullptr;
    vector<Symbol> symbols;
    vector<Scope*> children;



    Scope* getScope(const string& _identifier, const int occurrence = 0) {
        int count = 0;
        for (auto child : this->children) {
            if (child->name == _identifier) {
                if (count == occurrence)
                    return child;
                count++;
            }
        }
        return nullptr;
    }



    void print() {
        printScope(this, 0);
    }

    bool isEmpty(Scope* _scope) {
        if (!_scope)
            return true;

        if (!_scope->symbols.empty())
            return false;

        for (auto child : _scope->children)
            if (!isEmpty(child))
                return false;

        return true;
    }

    void printScope(Scope* _scope, int indent) {
        if (!_scope) return;
        
        if (isEmpty(_scope)) return;
        
        string spacing(indent * 2, ' ');
        cout << spacing << "[\033[0;90m" << _scope->name << "\033[0m]\n";
        
        for (const auto& symbol : _scope->symbols) {
            if (symbol.context == Context::error) {
                cout << spacing << " \033[1;31mWarning\033[0m: " << symbol.name << endl;
            }
            else {
                cout << spacing << " " << symbol.name << " : \033[0;36m";
                switch (symbol.context) {
                case Context::variable: cout << "variable "; break;
                case Context::function: cout << "function "; break;
                case Context::argument: cout << "argument "; break;
                case Context::parameter: cout << "parameter "; break;
                }

                if (!symbol.annotations.empty()) {
                    cout << "\033[0m[";
                    for (size_t i = 0; i < symbol.annotations.size(); ++i) {
                        cout << "\033[1;33m";
                        switch (symbol.annotations[i]) {
                        case _const: cout << "const"; break;
                        case _static: cout << "static"; break;
                        case _int: cout << "int"; break;
                        case _float: cout << "float"; break;
                        case _double: cout << "double"; break;
                        case _char: cout << "char"; break;
                        case _string: cout << "string"; break;
                        case _bool: cout << "bool"; break;
                        case _void: cout << "void"; break;
                        }
                        cout << "\033[0m";
                        if (i + 1 < symbol.annotations.size()) cout << ", ";
                    }
                    cout << "]";
                }

                cout << "\033[0m" << " (\033[0;91m" << symbol.type << "\033[0m)\n";


            }
        }
        cout << endl;
        
        for (auto child : _scope->children)
            printScope(child, indent + 1);
    }
};

