#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_map>
#include <stdexcept>
using namespace std;

enum class TokenType {
    T_FUNCTION, T_INT, T_FLOAT, T_STRING, T_BOOL,
    T_RETURN, T_IF, T_ELSE, T_WHILE, T_FOR,
    T_IDENTIFIER, T_INTLIT, T_FLOATLIT, T_STRINGLIT,
    T_PARENL, T_PARENR, T_BRACEL, T_BRACER, T_LBRACK, T_RBRACK,
    T_COMMA, T_SEMICOLON,
    T_ASSIGNOP, T_EQUALSOP, T_NEQ, T_LE, T_GE, T_LT, T_GT,
    T_PLUS, T_MINUS, T_MUL, T_DIV, T_MOD, T_AND, T_OR, T_NOT,
    T_EOF, T_UNKNOWN
};

struct Token {
    TokenType type;
    string lexeme;
    int line;
    Token(TokenType t = TokenType::T_UNKNOWN, string lex = "", int ln = 0)
        : type(t), lexeme(move(lex)), line(ln) {
    }
};

string tokenTypeToString(TokenType t) {
    switch (t) {
    case TokenType::T_FUNCTION: return "T_FUNCTION";
    case TokenType::T_INT: return "T_INT";
    case TokenType::T_FLOAT: return "T_FLOAT";
    case TokenType::T_STRING: return "T_STRING";
    case TokenType::T_BOOL: return "T_BOOL";
    case TokenType::T_RETURN: return "T_RETURN";
    case TokenType::T_IF: return "T_IF";
    case TokenType::T_ELSE: return "T_ELSE";
    case TokenType::T_WHILE: return "T_WHILE";
    case TokenType::T_FOR: return "T_FOR";
    case TokenType::T_IDENTIFIER: return "T_IDENTIFIER";
    case TokenType::T_INTLIT: return "T_INTLIT";
    case TokenType::T_FLOATLIT: return "T_FLOATLIT";
    case TokenType::T_STRINGLIT: return "T_STRINGLIT";
    case TokenType::T_PARENL: return "T_PARENL";
    case TokenType::T_PARENR: return "T_PARENR";
    case TokenType::T_BRACEL: return "T_BRACEL";
    case TokenType::T_BRACER: return "T_BRACER";
    case TokenType::T_LBRACK: return "T_LBRACK";
    case TokenType::T_RBRACK: return "T_RBRACK";
    case TokenType::T_COMMA: return "T_COMMA";
    case TokenType::T_SEMICOLON: return "T_SEMICOLON";
    case TokenType::T_ASSIGNOP: return "T_ASSIGNOP";
    case TokenType::T_EQUALSOP: return "T_EQUALSOP";
    case TokenType::T_NEQ: return "T_NEQ";
    case TokenType::T_LE: return "T_LE";
    case TokenType::T_GE: return "T_GE";
    case TokenType::T_LT: return "T_LT";
    case TokenType::T_GT: return "T_GT";
    case TokenType::T_PLUS: return "T_PLUS";
    case TokenType::T_MINUS: return "T_MINUS";
    case TokenType::T_MUL: return "T_MUL";
    case TokenType::T_DIV: return "T_DIV";
    case TokenType::T_MOD: return "T_MOD";
    case TokenType::T_AND: return "T_AND";
    case TokenType::T_OR: return "T_OR";
    case TokenType::T_NOT: return "T_NOT";
    case TokenType::T_EOF: return "T_EOF";
    default: return "T_UNKNOWN";
    }
}

class Lexer {
public:
    Lexer(const string& src) : src(src), pos(0), line(1) {
        initKeywords();
    }

    vector<Token> tokenize() {
        vector<Token> out;
        while (true) {
            skipWhitespaceAndComments();
            if (eof()) { out.emplace_back(TokenType::T_EOF, "", line); break; }
            char c = peek();
            if (isIdentifierStart(c)) {
                out.push_back(readIdentifierOrKeyword());
                continue;
            }
            if (isdigit(static_cast<unsigned char>(c))) {
                out.push_back(readNumber());
                continue;
            }
            if (c == '"') {
                out.push_back(readString());
                continue;
            }
            out.push_back(readOperatorOrSymbol());
        }
        return out;
    }

private:
    string src;
    size_t pos;
    int line;
    unordered_map<string, TokenType> keywords;

    void initKeywords() {
        keywords["fn"] = TokenType::T_FUNCTION;
        keywords["int"] = TokenType::T_INT;
        keywords["float"] = TokenType::T_FLOAT;
        keywords["string"] = TokenType::T_STRING;
        keywords["bool"] = TokenType::T_BOOL;
        keywords["return"] = TokenType::T_RETURN;
        keywords["if"] = TokenType::T_IF;
        keywords["else"] = TokenType::T_ELSE;
        keywords["while"] = TokenType::T_WHILE;
        keywords["for"] = TokenType::T_FOR;
    }

    bool eof() const { return pos >= src.size(); }
    char peek(int off = 0) const { return (pos + off < src.size()) ? src[pos + off] : '\0'; }
    char get() { char c = peek(); pos++; if (c == '\n') ++line; return c; }

    static bool isIdentifierStart(char c) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (c == '_') return true;
        if (isalpha(uc)) return true;
        if (uc >= 128) return true;
        return false;
    }
    static bool isIdentifierPart(char c) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (c == '_') return true;
        if (isalnum(uc)) return true;
        if (uc >= 128) return true;
        return false;
    }

    void skipWhitespaceAndComments() {
        while (!eof()) {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                get(); continue;
            }
            if (c == '/' && peek(1) == '/') {
                get(); get();
                while (!eof() && peek() != '\n') get();
                continue;
            }
            if (c == '/' && peek(1) == '*') {
                get(); get();
                while (!eof()) {
                    if (peek() == '*' && peek(1) == '/') { get(); get(); break; }
                    get();
                }
                continue;
            }
            break;
        }
    }

    Token readIdentifierOrKeyword() {
        int startLine = line;
        string s;
        s += get();
        while (!eof() && isIdentifierPart(peek())) s += get();
        auto it = keywords.find(s);
        if (it != keywords.end()) return Token(it->second, s, startLine);
        return Token(TokenType::T_IDENTIFIER, s, startLine);
    }

    Token readNumber() {
        int startLine = line;
        string s;
        while (!eof() && isdigit(static_cast<unsigned char>(peek()))) s += get();
        bool isFloat = false;
        if (peek() == '.' && isdigit(static_cast<unsigned char>(peek(1)))) {
            isFloat = true;
            s += get();
            while (!eof() && isdigit(static_cast<unsigned char>(peek()))) s += get();
        }
        if ((peek() == 'e' || peek() == 'E')) {
            char e = peek();
            if (isdigit(static_cast<unsigned char>(peek(1))) || (peek(1) == '+' || peek(1) == '-') && isdigit(static_cast<unsigned char>(peek(2)))) {
                isFloat = true;
                s += get();
                if (peek() == '+' || peek() == '-') s += get();
                while (!eof() && isdigit(static_cast<unsigned char>(peek()))) s += get();
            }
        }
        if (!eof() && isIdentifierPart(peek())) {
            string rest;
            while (!eof() && isIdentifierPart(peek())) rest += get();
            throw runtime_error("Invalid identifier starting with digit at line " + to_string(startLine) + ": number followed by letters: " + s + rest);
        }
        if (isFloat) return Token(TokenType::T_FLOATLIT, s, startLine);
        return Token(TokenType::T_INTLIT, s, startLine);
    }

    Token readString() {
        int startLine = line;
        if (get() != '"') throw runtime_error("readString called but no opening quote");
        string s;
        while (!eof()) {
            char c = get();
            if (c == '"') {
                return Token(TokenType::T_STRINGLIT, s, startLine);
            }
            if (c == '\\') {
                if (eof()) throw runtime_error("Unfinished escape sequence in string literal at line " + to_string(startLine));
                char esc = get();
                switch (esc) {
                case 'n': s.push_back('\n'); break;
                case 't': s.push_back('\t'); break;
                case 'r': s.push_back('\r'); break;
                case '\\': s.push_back('\\'); break;
                case '"': s.push_back('"'); break;
                case '\'': s.push_back('\''); break;
                case '0': s.push_back('\0'); break;
                default:
                    s.push_back(esc);
                }
            }
            else {
                s.push_back(c);
            }
        }
        throw runtime_error("Unterminated string literal starting at line " + to_string(startLine));
    }

    Token readOperatorOrSymbol() {
        int startLine = line;
        char c = peek();
        char n = peek(1);

        if (c == '=' && n == '=') { get(); get(); return Token(TokenType::T_EQUALSOP, "==", startLine); }
        if (c == '!' && n == '=') { get(); get(); return Token(TokenType::T_NEQ, "!=", startLine); }
        if (c == '<' && n == '=') { get(); get(); return Token(TokenType::T_LE, "<=", startLine); }
        if (c == '>' && n == '=') { get(); get(); return Token(TokenType::T_GE, ">=", startLine); }
        if (c == '&' && n == '&') { get(); get(); return Token(TokenType::T_AND, "&&", startLine); }
        if (c == '|' && n == '|') { get(); get(); return Token(TokenType::T_OR, "||", startLine); }

        switch (c) {
        case '=': get(); return Token(TokenType::T_ASSIGNOP, "=", startLine);
        case '<': get(); return Token(TokenType::T_LT, "<", startLine);
        case '>': get(); return Token(TokenType::T_GT, ">", startLine);
        case '!': get(); return Token(TokenType::T_NOT, "!", startLine);
        case '+': get(); return Token(TokenType::T_PLUS, "+", startLine);
        case '-': get(); return Token(TokenType::T_MINUS, "-", startLine);
        case '*': get(); return Token(TokenType::T_MUL, "*", startLine);
        case '/': get(); return Token(TokenType::T_DIV, "/", startLine);
        case '%': get(); return Token(TokenType::T_MOD, "%", startLine);
        case '(': get(); return Token(TokenType::T_PARENL, "(", startLine);
        case ')': get(); return Token(TokenType::T_PARENR, ")", startLine);
        case '{': get(); return Token(TokenType::T_BRACEL, "{", startLine);
        case '}': get(); return Token(TokenType::T_BRACER, "}", startLine);
        case '[': get(); return Token(TokenType::T_LBRACK, "[", startLine);
        case ']': get(); return Token(TokenType::T_RBRACK, "]", startLine);
        case ',': get(); return Token(TokenType::T_COMMA, ",", startLine);
        case ';': get(); return Token(TokenType::T_SEMICOLON, ";", startLine);
        default:
            string r(1, c);
            get();
            return Token(TokenType::T_UNKNOWN, r, startLine);
        }
    }
};


void runWithoutRegex(const string& filename) {
    ifstream in(filename);
    string src((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    try {
        Lexer L(src);
        auto tokens = L.tokenize();

        cout << "[\n";
        for (size_t i = 0; i < tokens.size(); ++i) {
            const Token& t = tokens[i];
            cout << "  " << tokenTypeToString(t.type);
            if (!t.lexeme.empty()) {
                string lex = t.lexeme;
                for (char& ch : lex) if (ch == '\n') ch = '\\', lex.insert(&ch - &lex[0] + 1, "n");
                cout << "(\"" << lex << "\")\n";
            }
        }
        cout << "]\n";
    }
    catch (const exception& ex) {
        cerr << "Lexer error: " << ex.what() << endl;
        return;
    }
}