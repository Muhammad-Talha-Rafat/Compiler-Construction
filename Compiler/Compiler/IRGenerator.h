// ir_generator.h
#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;


struct TAC {
    string op;
    string arg1;
    string arg2;
    string result;

    TAC() {}
    TAC(const string& _op, const string& _arg1 = "", const string& _arg2 = "", const string& _result = "")
        : op(_op), arg1(_arg1), arg2(_arg2), result(_result) {
    }
};


static string translate(const TAC& _operation) {

    if (_operation.op == "label") {
        if (_operation.result[0] == 'L' && _operation.result.length() == 2)
            return _operation.result + ":";
        else return "" + _operation.result + ":";
    }

    if (_operation.op == "goto")
        //                     Label
        return "goto " + _operation.result;

    if (_operation.op == "ifgoto")
        //              temp register                     Label
        return "if " + _operation.arg1 + " goto " + _operation.result;

    if (_operation.op == "param")
        return "param " + _operation.arg1;

    if (_operation.op == "arg")
        return "arg " + _operation.arg1;

    if (_operation.op == "call") {
        if (!_operation.result.empty())
            //                storing variable          function name        number of arguments
            return "call " + _operation.result + " " + _operation.arg1 + "[" + _operation.arg2 + "]";
        // calling a void function
        else return "call " + _operation.arg1 + "[" + _operation.arg2 + "]";
    }

    if (_operation.op == "return") {
        if (!_operation.arg1.empty())
            return "return " + _operation.arg1;
        else return "return" /* no arg for void function */;
    }

    if (_operation.op == "print") {
        return "print " + _operation.arg1;
    }

    // is it a binary operation?
    if (!_operation.result.empty() && !_operation.arg2.empty())
        return _operation.op + " " + _operation.result + " " + _operation.arg1 + " " + _operation.arg2;

    // is it a unirary operation?
    if (!_operation.result.empty() && _operation.arg2.empty())
        return _operation.op + " " + _operation.result + " " + _operation.arg1;

    return "<unknown>";
}





class IRGenerator
{
private:

    vector<TAC> operations;
    int temp_counter = 0;
    int label_counter = 0;

public:

    string get_temp() {
        return "$t" + to_string(temp_counter++);
    }

    string get_label() {
        return "L" + to_string(label_counter++);
    }


    void push(const TAC& _operation) {
        operations.push_back(_operation);
    }



    string convert(const string& _operator) {

        if (_operator == "ADDITION")       return "add";
        if (_operator == "SUBTRACTION")    return "sub";
        if (_operator == "MULTIPLICATION") return "mul";
        if (_operator == "DIVISION")       return "div";
        if (_operator == "MODULUS")        return "mod";

        if (_operator == "GREATERTHAN")    return "gt";
        if (_operator == "SMALLERTHAN")    return "lt";
        if (_operator == "GREATERorEQUAL") return "gte";
        if (_operator == "SMALLERorEQUAL") return "lte";
        if (_operator == "EQUALS")         return "eq";
        if (_operator == "NOTEQUAL")       return "neq";

        if (_operator == "AND_LOGIC")      return "and";
        if (_operator == "OR_LOGIC")       return "or";

        return "";
    }




    string translate_expression(Node* _node) {

        if (_node->type == "IDENTIFIER") {
            string result = get_temp();
            push(TAC("ld", _node->value, "", result));
            return result;
        }

        if (_node->type == "INTEGER" || _node->type == "DECIMAL" || _node->type == "CHARACTER" || _node->type == "STRLITERAL" || _node->type == "BOOLEAN")
            return _node->value;

        if (_node->type == "expression")
            // children[0] = <lPARENTHESIS>
            // children[1] = anything
            // children[2] = <rPARENTHESIS>
            return translate_expression(_node->children[1]);

        if (_node->type == "operation" || _node->type == "comparison")
        {
            Node* _left = _node->children[0];       // expression
            Node* _operator = _node->children[1];   // operator
            Node* _right = _node->children[2];      // expression

            string op = convert(_operator->type);
            string _arg1 = translate_expression(_left);
            string _arg2 = translate_expression(_right);

            string result = get_temp();
            push(TAC(op, _arg1, _arg2, result));
            return result;
        }

        if (_node->type == "increment") {
            string variable_name = _node->getValue("IDENTIFIER");
            string temp = get_temp();

            push(TAC("ld", variable_name, "", temp));
            push(TAC("add", temp, "1", temp));
            push(TAC("assign", temp, "", variable_name));

            return temp;
        }

        if (_node->type == "decrement") {
            string variable_name = _node->getValue("IDENTIFIER");
            string temp = get_temp();

            push(TAC("ld", variable_name, "", temp));
            push(TAC("sub", temp, "1", temp));
            push(TAC("assign", temp, "", variable_name));

            return temp;
        }


        if (_node->type == "function_call")
        {
            string function_name;
            vector<string> arguments;

            for (auto* child : _node->children) {
                if (child->type == "IDENTIFIER" && function_name.empty())
                    function_name = child->value;
                else if (child->type == "argument") {
                    string argument = translate_expression(child->children[0]);
                    arguments.push_back(argument);
                }
            }

            for (auto& argument : arguments)
                push(TAC("arg", argument));

            string result = get_temp();
            push(TAC("call", function_name, to_string(arguments.size()), result));
            return result;
        }

        return "";
    }




    string translate_conditions(Node* _conditions) {

        if (_conditions->children.size() == 1 && _conditions->children[0]->type == "comparison")
            return translate_expression(_conditions->children[0]);

        string _left;
        string _operator;

        for (size_t i = 0; i < _conditions->children.size(); i++)
        {
            Node* condition = _conditions->children[i];

            if (condition->type == "comparison") {
                string _right = translate_expression(condition);
                if (_left.empty()) _left = _right;
                else {
                    string temp = get_temp();
                    string op = convert(_operator);
                    push(TAC(op, _left, _right, temp));
                    _left = temp;
                    _operator.clear();
                }
            }
            else /* AND_LOGIC / OR_LOGIC */ _operator = condition->type;
        }

        return _left;
    }




    void translate_statement(Node* _statement) {

        if (_statement->type == "variable") {

            string _type;

            for (size_t i = 0; i < _statement->children.size(); i++) {

                Node* child = _statement->children[i];

                if (child->type == "TYPE")
                    _type = child->value;
                else if (child->type == "IDENTIFIER")
                {
                    string variable_name = child->value;

                    if (i + 2 < _statement->children.size() && _statement->children[i + 1]->type == "ASSIGN") {
                        Node* _expression = _statement->children[i + 2];
                        string temp = translate_expression(_expression);
                        push(TAC("assign", temp, "", variable_name));
                        i += 2;
                    }
                }
            }
            return;
        }


        if (_statement->type == "assignment")
        {
            string result = _statement->children[0]->value;
            // children[1] is always ASSIGN token
            Node* _expression = _statement->children[2];

            string _arg1 = translate_expression(_expression);
            push(TAC("assign", _arg1, "", result));

            return;
        }

        if (_statement->type == "increment")
        {
            string variable_name = _statement->getValue("IDENTIFIER");
            string temp = get_temp();

            push(TAC("ld", variable_name, "", temp));
            push(TAC("add", temp, "1", temp));
            push(TAC("assign", temp, "", variable_name));

            return;
        }

        if (_statement->type == "decrement")
        {
            string variable_name = _statement->getValue("IDENTIFIER");
            string temp = get_temp();

            push(TAC("ld", variable_name, "", temp));
            push(TAC("sub", temp, "1", temp));
            push(TAC("assign", temp, "", variable_name));

            return;
        }


        if (_statement->type == "output") {

            for (auto* child : _statement->children)
            {
                if (child->value == "cout") continue;
                else if (child->value == "<<") continue;
                else if (child->value == ";") continue;
                else if (child->value == "endl") push(TAC("print", "endl"));
                else {
                    string result = translate_expression(child);
                    push(TAC("print", result));
                }
            }

            return;
        }


        if (_statement->type == "call_statement") {
            string function_name = _statement->getValue("IDENTIFIER");
            vector<string> arguments;

            for (auto* child : _statement->children)
                if (child->type == "argument")
                    arguments.push_back(translate_expression(child->children[0]));

            for (auto& argument : arguments)
                push(TAC("param", argument));

            push(TAC("call", function_name, to_string(arguments.size())));

            return;
        }


        if (_statement->type == "return") {
            string _expression = translate_expression(_statement->children[1]);
            push(TAC("return", _expression));
            return;
        }


        if (_statement->type == "decision") {

            string exit_label = get_label();

            for (auto* _block : _statement->children) {

                if (_block->type == "if_block" || _block->type == "else_if_block") {

                    Node* _conditions = nullptr;
                    Node* block_node = nullptr;

                    for (auto* child : _block->children) {
                        if (child->type == "conditions")
                            _conditions = child;
                        if (child->type == "block")
                            block_node = child;
                    }

                    string _arg1 = translate_conditions(_conditions);
                    string block_label = get_label();

                    push(TAC("ifgoto", _arg1, "", block_label));
                    push(TAC("label", "", "", block_label));

                    if (block_node) {
                        for (auto* child : block_node->children)
                            translate_statement(child);
                    }

                    push(TAC("goto", "", "", exit_label));
                }
                else if (_block->type == "else_block") {
                    // else_block has a block child
                    Node* blockNode = nullptr;
                    for (auto* child : _block->children) {
                        if (child && child->type == "block")
                            blockNode = child;
                    }

                    string bodyLabel = get_label();
                    push(TAC("label", "", "", bodyLabel));

                    if (blockNode) {
                        for (auto* child : blockNode->children)
                            translate_statement(child);
                    }

                    push(TAC("goto", "", "", exit_label));
                }
            }
            // end label
            push(TAC("label", "", "", exit_label));
            return;
        }

        // for_loop
        if (_statement->type == "for_loop") {
            // children: for, lparen, variable/assignment, ;, conditions, ;, update, rparen, lbrace, block, rbrace
            Node* _init = nullptr;
            Node* _conditions = nullptr;
            Node* _update = nullptr;
            Node* _block = nullptr;

            for (auto* child : _statement->children) {
                if (!child) continue;
                if ((child->type == "variable" || child->type == "assignment") && !_init) _init = child;
                if (child->type == "conditions" && !_conditions) _conditions = child;
                if (child->type == "update" && !_update) _update = child;
                if (child->type == "block" && !_block) _block = child;
            }

            if (_init) translate_statement(_init);

            string startLabel = get_label();
            string bodyLabel = get_label();
            string endLabel = get_label();

            push(TAC("label", "", "", startLabel));

            string condTemp = translate_conditions(_conditions);
            push(TAC("ifgoto", condTemp, "", bodyLabel));
            push(TAC("goto", "", "", endLabel));
            push(TAC("label", "", "", bodyLabel));

            if (_block) {
                for (auto* st : _block->children)
                    translate_statement(st);
            }

            if (_update) {

                for (auto* child : _update->children) {

                    if (child->type == "increment") {
                        string name = child->getValue("IDENTIFIER");
                        string temp = get_temp();
                        push(TAC("ld", name, "", temp));
                        push(TAC("add", temp, "1", temp));
                        push(TAC("assign", temp, "", name));
                    }

                    else if (child->type == "decrement") {
                        string name = child->getValue("IDENTIFIER");
                        string temp = get_temp();
                        push(TAC("ld", name, "", temp));
                        push(TAC("sub", temp, "1", temp));
                        push(TAC("assign", temp, "", name));
                    }

                    else if (child->type == "assignment") {
                        string variable_name = child->children[0]->value; 
                        string _operator = convert(child->children[1]->type);
                        Node* _expression = child->children[2];

                        string right = translate_expression(_expression);
                        string temp = get_temp();

                        if (_operator == "=")
                            push(TAC("assign", right, "", variable_name));
                        else {
                            _operator = string(1, _operator[0]);
                            push(TAC("ld", variable_name, "", temp));
                            push(TAC(_operator, temp, right, temp));
                            push(TAC("assign", temp, "", variable_name));
                        }
                    }
                }
            }
            push(TAC("goto", "", "", startLabel));
            push(TAC("label", "", "", endLabel));
            return;
        }

        // while_loop
        if (_statement->type == "while_loop")
        {
            Node* _conditions = nullptr;
            Node* _block = nullptr;

            for (auto* child : _statement->children) {
                if (!child) continue;
                if (child->type == "conditions") _conditions = child;
                if (child->type == "block") _block = child;
            }

            string startLabel = get_label();
            string bodyLabel = get_label();
            string endLabel = get_label();

            push(TAC("label", "", "", startLabel));

            string conditions = translate_conditions(_conditions);
            push(TAC("ifgoto", conditions, "", bodyLabel));
            push(TAC("goto", "", "", endLabel));

            push(TAC("label", "", "", bodyLabel));

            if (_block) {
                for (auto* child : _block->children)
                    translate_statement(child);
            }

            push(TAC("goto", "", "", startLabel));
            push(TAC("label", "", "", endLabel));
            return;
        }

        // do_while_loop
        if (_statement->type == "do_while_loop")
        {
            Node* _block = nullptr;
            Node* _conditions = nullptr;

            for (auto* child : _statement->children) {
                if (!child) continue;
                if (child->type == "block") _block = child;
                if (child->type == "conditions") _conditions = child;
            }

            string bodyLabel = get_label();
            string endLabel = get_label();

            push(TAC("label", "", "", bodyLabel));

            if (_block) {
                for (auto* child : _block->children)
                    translate_statement(child);
            }

            if (_conditions) {
                string conditions = translate_conditions(_conditions);
                push(TAC("ifgoto", conditions, "", bodyLabel));
            }

            push(TAC("label", "", "", endLabel));
            return;
        }

        // any sort off block
        if (_statement->type == "block") {
            for (auto* statement : _statement->children)
                translate_statement(statement);
            return;
        }

        if (_statement->type == "function") {
            translate_function(_statement);
            return;
        }

        if (_statement->type == "function_call") {
            string temp = translate_expression(_statement);
            return;
        }

        for (auto* child : _statement->children) {
            if (!child) continue;
            translate_statement(child);
        }
    }


    void translate_function(Node* _function)
    {
        string function_name = _function->getValue("IDENTIFIER");
        if (function_name.empty()) function_name = "main";

        push(TAC("label", "", "", function_name));

        Node* _block = nullptr;
        for (auto* child : _function->children) {
            if (child->type == "block") {
                _block = child;
                break;
            }
        }

        if (_block) {
            for (auto* child : _block->children)
                translate_statement(child);
        }
    }


    void generate(Node* program) {
        for (auto* child : program->children) {
            if (child->type == "declarations") {
                for (auto* declaration : child->children) {
                    if (!declaration) continue;
                    translate_statement(declaration);
                }
            }
            // you encountered global variables
            else translate_statement(child);
        }
    }


    void printCode() {
        for (const auto& operation : operations) {
            cout << translate(operation) << '\n';
        }
    }

};

