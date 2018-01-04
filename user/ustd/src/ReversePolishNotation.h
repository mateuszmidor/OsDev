/**
 *   @file: ReversePolishNotation.h
 *
 *   @date: Jan 3, 2018
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_REVERSEPOLISHNOTATION_H_
#define USER_USTD_SRC_REVERSEPOLISHNOTATION_H_

#include "Optional.h"
#include "StringUtils.h"
#include "Map.h"

namespace ustd {
namespace rpn {

/**
 * TOKENIZER PART
 */
enum class TokenType {
    PLUS, MINUS, MUL, DIV, POWER,           // +, -, *, /, ^
    F_NEG, F_ABS, F_SQRT, F_SIN, F_COS,     // neg, abs, sqrt, sin, cos
    L_PAR, R_PAR,                           // (, )
    NUMBER, VARIABLE,                       // 12.34, PI
    INVALID
};

struct Token {
    Token(TokenType type, const string& value) : type(type), value(value) { }
    TokenType   type;
    string      value;
};

/**
 * @brief   Break a math equation into a stream of tokens
 */
class Tokenizer {
public:
    Optional<vector<Token>> tokenize_equation(const string& equation);

    string to_string(const vector<Token>& tokens);

private:
    bool try_fetch_token(string& s, const string& token);

    bool try_fetch_variable(string& s, string& out);

    bool try_fetch_number(string& s, string& out);

    void eat_whitespaces(string& s);

    bool isalpha(char c) { return ((c >='a' && c <= 'z') || (c >='A' && c <= 'Z')); }

    bool isdigit(char c) { return (c >='0' && c <= '9'); }

    bool iswspace(char c) { return (c == ' ' || c == '\t'); }

    bool isalnum(char c) { return isalpha(c) || isdigit(c); }

};

/**
 * @brief   Convert stream of tokens into Reverse Polish Notation
 */
class RpnBuilder {
public:
    Optional<vector<Token>> build(vector<Token> tokens);

    string to_string(const vector<Token>& tokens);

private:
    int priority(const Token& t);

    bool is_right_associative(const Token& t);

    void preprocess_unary_tokens(vector<Token>& tokens);

    bool is_unary(TokenType curr, TokenType prev);
};

/**
 * @brief   Calculate result of Reverse Polish Notation
 */
class RpnEvaluator {
public:
    Optional<double> eval(const vector<Token>& onp);

    void define(const string& name, double value) { names[name] = value; }

private:
    Optional<double> get_value_for_name(const string& name);

    Map<string, double> names;
};

class Calculator {
public:
    Calculator();
    Optional<bool> parse(const string& eq);
    void define(const string& name, double value);
    Optional<double> calc();

private:
    vector<Token> onp;
    RpnEvaluator evaluator;
};

} /* namespace rpn */
} /* namespace ustd */

#endif /* USER_USTD_SRC_REVERSEPOLISHNOTATION_H_ */
