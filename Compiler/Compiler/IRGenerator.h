// ir_generator.h
#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;

struct IR {
    string op;
    string arg1;
    string arg2;
    string result;

    IR() {}
    IR(const string& _op, const string& _a1 = "", const string& _a2 = "", const string& _res = "")
        : op(_op), arg1(_a1), arg2(_a2), result(_res) {
    }
};

// pretty print an IR instruction
static string irToString(const IR& ir) {
    // handle different forms
    if (ir.op == "label") return ir.result + ":";
    if (ir.op == "goto") return "goto " + ir.result;
    if (ir.op == "ifgoto") return "if " + ir.arg1 + " goto " + ir.result;
    if (ir.op == "param") return "param " + ir.arg1;
    if (ir.op == "call") {
        if (!ir.result.empty())
            return ir.result + " = call " + ir.arg1 + ", " + ir.arg2;
        else
            return "call " + ir.arg1 + ", " + ir.arg2;
    }
    if (ir.op == "return") {
        if (!ir.arg1.empty()) return "return " + ir.arg1;
        else return "return";
    }
    if (ir.op == "print") {
        return "print " + ir.arg1;
    }
    // binary op: result = arg1 op arg2  (op spelled out)
    if (!ir.result.empty() && !ir.arg2.empty())
        return ir.result + " = " + ir.arg1 + " " + ir.op + " " + ir.arg2;
    // unary: result = op arg1
    if (!ir.result.empty() && ir.arg2.empty())
        return ir.result + " = " + ir.op + " " + ir.arg1;
    // fallback
    ostringstream ss;
    ss << ir.op << " " << ir.arg1 << " " << ir.arg2 << " -> " << ir.result;
    return ss.str();
}

// ---------------------------
// IRGenerator
// ---------------------------
class IRGenerator {
    vector<IR> code;
    int tcount = 0;
    int lcount = 0;

public:
    IRGenerator() = default;

    // generate unique temps and labels
    string newTemp() {
        return "$t" + to_string(tcount++);
    }
    string newLabel() {
        return "L" + to_string(lcount++);
    }

    // emit helper
    void emit(const IR& inst) { code.push_back(inst); }

    // map operator token types to TAC op names (for arithmetic & comparisons)
    string mapBinOp(const string& opType, const string& opValue = "") {
        // arithmetic
        if (opType == "ADDITION" || opValue == "+") return "add";
        if (opType == "SUBTRACTION" || opValue == "-") return "sub";
        if (opType == "MULTIPLICATION" || opValue == "*") return "mul";
        if (opType == "DIVISION" || opValue == "/") return "div";
        if (opType == "MODULUS" || opValue == "%") return "mod";
        // comparisons
        if (opType == "GREATERTHAN" || opValue == ">") return "gt";
        if (opType == "SMALLERTHAN" || opValue == "<") return "lt";
        if (opType == "GREATERorEQUAL" || opValue == ">=") return "gte";
        if (opType == "SMALLERorEQUAL" || opValue == "<=") return "lte";
        if (opType == "EQUALS" || opValue == "==") return "eq";
        if (opType == "NOTEQUAL" || opValue == "!=") return "neq";
        // logical
        if (opType == "AND_LOGIC" || opValue == "&&") return "and";
        if (opType == "OR_LOGIC" || opValue == "||") return "or";
        // fallback
        return opValue.empty() ? opType : opValue;
    }

    // ---------- Expression generation ----------
    // Returns the name of the temp/var that holds the computed value
    string genExpression(Node* node) {
        if (!node) return "";

        // direct literal / identifier nodes
        if (node->children.empty()) {
            if (node->type == "INTEGER" || node->type == "STRLITERAL") {
                // For literals, we can return immediate string (but many backends prefer temps)
                string t = newTemp();
                emit(IR("load", node->value, "", t));       // t = load literal
                return t;
            }
            if (node->type == "IDENTIFIER") {
                // identifiers: return their name (we'll use them directly)
                return node->value;
            }
            // other leaf types might be KEYWORD: endl etc.
            if (node->type == "KEYWORD" && node->value == "endl") {
                // treat as literal token "endl"
                string t = newTemp();
                emit(IR("load", "endl", "", t));
                return t;
            }
        }

        // parenthesized expression: child may be ( lparen, inner, rparen )
        if (node->type == "expression") {
            // find the child that is an operation or literal/identifier/expr
            for (auto* c : node->children) {
                if (!c) continue;
                if (c->type == "operation" || c->type == "expression" || c->type == "function_call" || c->type == "IDENTIFIER" || c->type == "INTEGER" || c->type == "STRLITERAL") {
                    return genExpression(c);
                }
            }
            // fallback: recursively handle first child
            if (!node->children.empty()) return genExpression(node->children[0]);
        }

        // operation node: children: left, operator, right
        if (node->type == "operation") {
            // expected 3 children: left, operator, right
            Node* left = nullptr;
            Node* right = nullptr;
            Node* opNode = nullptr;
            if (node->children.size() >= 3) {
                left = node->children[0];
                opNode = node->children[1];
                right = node->children[2];
            }
            else {
                // try to find them heuristically
                if (!node->children.empty()) left = node->children[0];
                if (node->children.size() >= 2) right = node->children[1];
            }
            string leftVal = genExpression(left);
            string rightVal = genExpression(right);
            string op = mapBinOp(opNode ? opNode->type : "", opNode ? opNode->value : "");

            // produce temp and emit
            string dest = newTemp();
            emit(IR(op, leftVal, rightVal, dest));
            return dest;
        }

        // function_call used as expression (returns value)
        if (node->type == "function_call") {
            // first child is identifier (function name), subsequent children are arguments
            string funcName;
            vector<string> args;
            for (auto* c : node->children) {
                if (!c) continue;
                if (c->type == "IDENTIFIER" && funcName.empty()) {
                    funcName = c->value;
                    continue;
                }
                if (c->type == "argument") {
                    // argument node has one child which is expression/identifier
                    if (!c->children.empty()) {
                        string argVal = genExpression(c->children[0]);
                        args.push_back(argVal);
                    }
                }
                else if (c->type == "IDENTIFIER") {
                    // sometimes arguments might be directly attached
                    args.push_back(c->value);
                }
            }
            // param for each arg
            for (auto& a : args) emit(IR("param", a, "", ""));
            string dest = newTemp();
            emit(IR("call", funcName, to_string((int)args.size()), dest));
            return dest;
        }

        // fallback: try to descend into the first child
        if (!node->children.empty()) return genExpression(node->children[0]);

        // if nothing, return empty
        return "";
    }

    // ---------- Condition generation ----------
    // returns a temp that holds boolean (0/1)
    string genComparison(Node* cmp) {
        // cmp children: left, operator, right
        if (!cmp) return "";
        Node* left = nullptr;
        Node* op = nullptr;
        Node* right = nullptr;
        if (cmp->children.size() >= 3) {
            left = cmp->children[0];
            op = cmp->children[1];
            right = cmp->children[2];
        }
        else {
            // fallback: attempt
            if (!cmp->children.empty()) left = cmp->children[0];
        }
        string l = genExpression(left);
        string r = genExpression(right);
        string opName = mapBinOp(op ? op->type : "", op ? op->value : "");
        string dest = newTemp();
        emit(IR(opName, l, r, dest));
        return dest;
    }

    // genConditions handles simple lists and logical operators (AND/OR)
    string genConditions(Node* cond) {
        if (!cond) return "";

        // If a single comparison child
        if (cond->children.size() == 1 && cond->children[0]->type == "comparison") {
            return genComparison(cond->children[0]);
        }

        // Otherwise, attempt left op right flattening:
        // For example children: comparison / operator / comparison / operator / comparison ...
        // We'll fold left-to-right producing temps and combining with 'and'/'or'
        string leftTemp;
        string opType;
        for (size_t i = 0; i < cond->children.size(); ++i) {
            Node* c = cond->children[i];
            if (!c) continue;
            if (c->type == "comparison") {
                string compTemp = genComparison(c);
                if (leftTemp.empty()) leftTemp = compTemp;
                else {
                    // combine leftTemp and compTemp with previous opType
                    string dest = newTemp();
                    string opName = mapBinOp(opType, opType);
                    emit(IR(opName, leftTemp, compTemp, dest));
                    leftTemp = dest;
                    opType.clear();
                }
            }
            else {
                // operator token node e.g., AND_LOGIC or OR_LOGIC
                opType = c->type;
            }
        }
        if (!leftTemp.empty()) return leftTemp;
        // fallback: generate for first child
        if (!cond->children.empty()) return genConditions(cond->children[0]);
        return "";
    }

    // ---------- Statement generation ----------
    void genStatement(Node* stmt) {
        if (!stmt) return;

        // variable declaration (covers: int a; int a = expr; int a,b,c; const int a = 7;)
        if (stmt->type == "variable") {
            // If multiple identifiers separated by commas, children will contain multiple IDENTIFIER nodes
            // We'll walk children and when we find an IDENTIFIER followed optionally by ASSIGN + expression, emit assign/load
            string currentType;
            for (size_t i = 0; i < stmt->children.size(); ++i) {
                Node* c = stmt->children[i];
                if (!c) continue;
                if (c->type == "TYPE") currentType = c->value;
                else if (c->type == "IDENTIFIER") {
                    string varName = c->value;
                    // check next nodes for ASSIGN
                    if (i + 2 < stmt->children.size() && stmt->children[i + 1]->type == "ASSIGN") {
                        Node* rhs = stmt->children[i + 2];
                        string rhsTemp = genExpression(rhs);
                        // assign rhsTemp to varName
                        emit(IR("assign", rhsTemp, "", varName));
                        i += 2; // skip assign and rhs
                    }
                    else {
                        // uninitialized, optionally emit a 'alloc' or default load 0
                        // we'll skip explicit alloc and just leave it; optionally do: var = 0
                    }
                }
            }
            return;
        }

        // assignment statement (a = expr;)
        if (stmt->type == "assignment") {
            // children: IDENTIFIER ASSIGN expr
            string left;
            Node* rhs = nullptr;
            for (auto* c : stmt->children) {
                if (!c) continue;
                if (c->type == "IDENTIFIER" && left.empty()) left = c->value;
                if (c->type != "IDENTIFIER" && c->type != "ASSIGN") rhs = c; // crude
            }
            if (!left.empty() && rhs) {
                string r = genExpression(rhs);
                emit(IR("assign", r, "", left));
            }
            return;
        }

        // output (cout << ...;)
        if (stmt->type == "output") {
            // children might list tokens/values in order: STRLITERAL, IDENTIFIER, KEYWORD(endl), etc.
            vector<string> parts;
            for (auto* c : stmt->children) {
                if (!c) continue;
                if (c->type == "STRLITERAL") {
                    // create a temp for the literal or treat as immediate param
                    // We'll pass literal directly as param
                    parts.push_back(c->value);
                }
                else if (c->type == "IDENTIFIER") {
                    parts.push_back(genExpression(c)); // returns var name or temp
                }
                else if (c->type == "KEYWORD" && c->value == "endl") {
                    parts.push_back("endl");
                }
                else {
                    // skip SHIFT tokens etc.
                }
            }
            // emit params and call print
            for (auto& p : parts) emit(IR("param", p, "", ""));
            emit(IR("call", "cout", to_string((int)parts.size()), "")); // void call
            return;
        }

        // call_statement (function call as statement)
        if (stmt->type == "call_statement") {
            // same structure as function_call but no result stored
            string funcName;
            vector<string> args;
            for (auto* c : stmt->children) {
                if (!c) continue;
                if (c->type == "IDENTIFIER" && funcName.empty()) { funcName = c->value; continue; }
                if (c->type == "argument") {
                    if (!c->children.empty()) args.push_back(genExpression(c->children[0]));
                }
            }
            for (auto& a : args) emit(IR("param", a, "", ""));
            emit(IR("call", funcName, to_string((int)args.size()), ""));
            return;
        }

        // return
        if (stmt->type == "return") {
            // child may be INTEGER or expression or empty for void
            if (!stmt->children.empty()) {
                string retv = genExpression(stmt->children[0]);
                emit(IR("return", retv, "", ""));
            }
            else {
                emit(IR("return", "", "", ""));
            }
            return;
        }

        // decision (if / else if / else) - top-level container
        if (stmt->type == "decision") {
            // iterate children and find if_block / else_if_block / else_block
            string endLabel = newLabel();
            // We'll generate: check each branch, jump to body label if true else continue, after each body goto endLabel
            for (auto* branch : stmt->children) {
                if (!branch) continue;
                if (branch->type == "if_block" || branch->type == "else_if_block") {
                    // find conditions and block
                    Node* condNode = nullptr;
                    Node* blockNode = nullptr;
                    for (auto* ch : branch->children) {
                        if (!ch) continue;
                        if (ch->type == "conditions" && !condNode) condNode = ch;
                        if (ch->type == "block" && !blockNode) blockNode = ch;
                        // sometimes block node is wrapped or appears later — find first block
                        if (ch->type == "block") blockNode = ch;
                    }
                    string condTemp = condNode ? genConditions(condNode) : "";
                    string bodyLabel = newLabel();
                    if (!condTemp.empty()) {
                        emit(IR("ifgoto", condTemp, "", bodyLabel));
                    }
                    else {
                        // if there's no condition (shouldn't happen for if), just goto body
                        emit(IR("goto", "", "", bodyLabel));
                    }
                    // if condition false, execution will fallthrough to next branch check
                    // emit body label and body
                    emit(IR("label", "", "", bodyLabel));
                    // generate statements in block
                    if (blockNode) {
                        for (auto* st : blockNode->children) genStatement(st);
                    }
                    // after body, jump to end
                    emit(IR("goto", "", "", endLabel));
                }
                else if (branch->type == "else_block") {
                    // else_block has a block child
                    Node* blockNode = nullptr;
                    for (auto* ch : branch->children) if (ch && ch->type == "block") blockNode = ch;
                    string bodyLabel = newLabel();
                    emit(IR("label", "", "", bodyLabel));
                    if (blockNode) {
                        for (auto* st : blockNode->children) genStatement(st);
                    }
                    emit(IR("goto", "", "", endLabel));
                }
                else {
                    // ignore other children
                }
            }
            // end label
            emit(IR("label", "", "", endLabel));
            return;
        }

        // for_loop
        if (stmt->type == "for_loop") {
            // children: variable (init), ;, conditions, ;, update, rparen, lbrace, block
            Node* init = nullptr;
            Node* cond = nullptr;
            Node* update = nullptr;
            Node* block = nullptr;
            for (auto* c : stmt->children) {
                if (!c) continue;
                if (c->type == "variable" && !init) init = c;
                if (c->type == "conditions" && !cond) cond = c;
                if (c->type == "update" && !update) update = c;
                if (c->type == "block" && !block) block = c;
            }
            // init
            if (init) genStatement(init);
            string startLabel = newLabel();
            string bodyLabel = newLabel();
            string endLabel = newLabel();
            emit(IR("label", "", "", startLabel));
            // cond
            if (cond) {
                string condTemp = genConditions(cond);
                emit(IR("ifgoto", condTemp, "", bodyLabel));
                emit(IR("goto", "", "", endLabel));
            }
            else {
                // no condition => infinite loop -> goto body
                emit(IR("goto", "", "", bodyLabel));
            }
            emit(IR("label", "", "", bodyLabel));
            // body
            if (block) {
                for (auto* st : block->children) genStatement(st);
            }
            // update (e.g., increment)
            if (update) {
                // update children like increment node
                for (auto* u : update->children) {
                    if (!u) continue;
                    if (u->type == "increment") {
                        // increment child IDENTIFIER
                        if (!u->children.empty()) {
                            Node* id = u->children[0];
                            if (id && id->type == "IDENTIFIER") {
                                // id = id + 1
                                string idv = id->value;
                                string tmp = newTemp();
                                emit(IR("add", idv, "1", tmp));
                                emit(IR("assign", tmp, "", idv));
                            }
                        }
                    }
                    else if (u->type == "decrement") {
                        if (!u->children.empty()) {
                            Node* id = u->children[0];
                            if (id && id->type == "IDENTIFIER") {
                                string idv = id->value;
                                string tmp = newTemp();
                                emit(IR("sub", idv, "1", tmp));
                                emit(IR("assign", tmp, "", idv));
                            }
                        }
                    }
                    else {
                        // other update forms could be handled here
                    }
                }
            }
            emit(IR("goto", "", "", startLabel));
            emit(IR("label", "", "", endLabel));
            return;
        }

        // while_loop
        if (stmt->type == "while_loop") {
            Node* cond = nullptr;
            Node* block = nullptr;
            for (auto* c : stmt->children) {
                if (!c) continue;
                if (c->type == "conditions") cond = c;
                if (c->type == "block") block = c;
            }
            string startLabel = newLabel();
            string bodyLabel = newLabel();
            string endLabel = newLabel();
            emit(IR("label", "", "", startLabel));
            if (cond) {
                string condt = genConditions(cond);
                emit(IR("ifgoto", condt, "", bodyLabel));
                emit(IR("goto", "", "", endLabel));
            }
            else {
                emit(IR("goto", "", "", bodyLabel));
            }
            emit(IR("label", "", "", bodyLabel));
            if (block) for (auto* st : block->children) genStatement(st);
            emit(IR("goto", "", "", startLabel));
            emit(IR("label", "", "", endLabel));
            return;
        }

        // do_while_loop
        if (stmt->type == "do_while_loop") {
            Node* block = nullptr;
            Node* cond = nullptr;
            for (auto* c : stmt->children) {
                if (!c) continue;
                if (c->type == "block") block = c;
                if (c->type == "conditions") cond = c;
            }
            string bodyLabel = newLabel();
            string endLabel = newLabel();
            emit(IR("label", "", "", bodyLabel));
            if (block) for (auto* st : block->children) genStatement(st);
            // evaluate condition
            if (cond) {
                string condt = genConditions(cond);
                emit(IR("ifgoto", condt, "", bodyLabel));
            }
            emit(IR("label", "", "", endLabel));
            return;
        }

        // block: iterate inner statements
        if (stmt->type == "block") {
            for (auto* st : stmt->children) genStatement(st);
            return;
        }

        // function_call used as statement, but some call nodes might be directly here
        if (stmt->type == "function_call") {
            // treat as expression but drop result
            string tmp = genExpression(stmt);
            // if result exists and is temporary, we can ignore it
            return;
        }

        // function (definition inside top-level generate will be handled in genFunction)
        if (stmt->type == "function") {
            genFunction(stmt);
            return;
        }

        // fallback: try descending into children and generate for each
        for (auto* c : stmt->children) {
            if (!c) continue;
            genStatement(c);
        }
    }

    // ---------- Function generation ----------
    void genFunction(Node* func) {
        if (!func) return;
        // expected children: TYPE, IDENTIFIER, parameters..., block
        string fname;
        for (auto* c : func->children) {
            if (!c) continue;
            if (c->type == "IDENTIFIER" && fname.empty()) {
                fname = c->value;
                break;
            }
        }
        if (fname.empty()) {
            // maybe function uses KEYWORD: main etc.
            for (auto* c : func->children) if (c->type == "KEYWORD" && fname.empty()) fname = c->value;
        }
        // function label
        emit(IR("label", "", "", fname));
        // emit function body (block)
        Node* block = nullptr;
        for (auto* c : func->children) if (c->type == "block") { block = c; break; }
        if (block) {
            for (auto* st : block->children) genStatement(st);
        }
        // ensure function ends with return (for void, a bare return)
        // we don't force a return here; assume AST has return if needed
    }

    // ---------- Entry: generate for whole program ----------
    void generate(Node* program) {
        if (!program) return;
        // top-level expects "program" or "declarations" with function nodes
        for (auto* top : program->children) {
            if (!top) continue;
            if (top->type == "declarations" || top->type == "program") {
                for (auto* decl : top->children) {
                    if (!decl) continue;
                    genStatement(decl);
                }
            }
            else {
                genStatement(top);
            }
        }
    }

    // retrieve generated code
    const vector<IR>& getCode() const { return code; }

    // print generated IR
    void printCode(ostream& os = cout) const {
        for (const auto& inst : code) {
            os << irToString(inst) << '\n';
        }
    }
};
