/**
 *   @file: ReversePolishNotation.cpp
 *
 *   @date: Jan 3, 2018
 * @author: Mateusz Midor
 */

#include <stack>
#include "ReversePolishNotation.h"
#include "Math.h"
#include "StringUtils.h"

namespace cstd {
namespace ustd {
namespace rpn {

Optional<vector<Token>> Tokenizer::tokenize_equation(const string& equation) {
    string s = StringUtils::to_lower_case(equation);

    vector<Token> result;
    string value {};

    while (!s.empty()) {
        eat_whitespaces(s);

        if (try_fetch_token(s, "neg"))
            result.emplace_back(TokenType::F_NEG, "neg");
        else if (try_fetch_token(s, "abs"))
            result.emplace_back(TokenType::F_ABS, "abs");
        else if (try_fetch_token(s, "sqrt"))
            result.emplace_back(TokenType::F_SQRT, "sqrt");
        else if (try_fetch_token(s, "sin"))
            result.emplace_back(TokenType::F_SIN, "sin");
        else if (try_fetch_token(s, "cos"))
            result.emplace_back(TokenType::F_COS, "cos");
        else if (try_fetch_token(s, "+"))
            result.emplace_back(TokenType::PLUS, "+");
        else if (try_fetch_token(s, "-"))
            result.emplace_back(TokenType::MINUS, "-");
        else if (try_fetch_token(s, "*"))
            result.emplace_back(TokenType::MUL, "*");
        else if (try_fetch_token(s, "/"))
            result.emplace_back(TokenType::DIV, "/");
        else if (try_fetch_token(s, "^"))
            result.emplace_back(TokenType::POWER, "^");
        else if (try_fetch_token(s, "("))
            result.emplace_back(TokenType::L_PAR, "(");
        else if (try_fetch_token(s, ")"))
            result.emplace_back(TokenType::R_PAR, ")");
        else if (try_fetch_number(s, value))
            result.emplace_back(TokenType::NUMBER, value);
        else if (try_fetch_variable(s, value))
            result.emplace_back(TokenType::VARIABLE, value);
        else
            return {"unknown token at the beginning: " + s};
    }

    return {std::move(result)};
}

string Tokenizer::to_string(const vector<Token>& tokens) {
    string result {};
    for (auto t : tokens)
        result += t.value + " ";
    return std::move(result);
}

bool Tokenizer::try_fetch_token(string& s, const string& token) {
    auto token_len = token.length();

    if (s.length() < token_len)
        return false;

    if (s.compare(0, token_len, token) != 0)
        return false;

    s.erase(0, token_len);
    return true;
}

bool Tokenizer::try_fetch_variable(string& s, string& out) {
    if (s.empty())
        return false;

    auto stop = s.cbegin();

    // an identifier starts with a  letter
    if (!isalpha(*stop))
        return false;

    // followed by more letters or digits ot underscores
    while (stop != s.end() && ((isalnum(*stop) || *stop == '_')))
        stop++;

    out = {s.cbegin(), stop};
    s.erase(s.cbegin(), stop);

    return true;
}

/**
 * numbers are always positive, negative number is treated as negation of positive number eg. -5 -> neg(5)
 * this simplifies distinguishing unary "-" from binary "-" and so simplifies processing
 */
bool Tokenizer::try_fetch_number(string& s, string& out) {
    const char FLOATING_POINT_MARKER {'.'};

    if (s.empty())
        return false;

    auto stop = s.cbegin();

    // positive number starts with a digit
    if (!isdigit(*stop))
        return false;

    // followed by any number of digits and perhaps floating point marker
    stop++;
    bool floating_point_detected = false;
    while (stop != s.end()) {
        if (isdigit(*stop))
            stop++;
        else if (((*stop) == FLOATING_POINT_MARKER) && (!floating_point_detected)) {
            floating_point_detected = true;
            stop++;
        }
        else
            break;
    }

    out = {s.cbegin(), stop};
    s.erase(s.cbegin(), stop);

    return true;
}

void Tokenizer::eat_whitespaces(string& s) {
    auto stop = s.cbegin();
    while ((stop != s.cend()) && iswspace(*stop))
        stop++;

    s.erase(s.cbegin(), stop);
}

Optional<vector<Token>> RpnBuilder::build(vector<Token> tokens) {
    if (tokens.empty())
        return {"empty input"};

    preprocess_unary_tokens(tokens);

    vector<Token> result;
    std::stack<Token> op_stack;

    // based on http://eduinf.waw.pl/inf/alg/001_search/0102.php, credits to mgr Jerzy Wałaszek
    for (const auto& t : tokens) {
        switch (t.type) {
        case TokenType::L_PAR:
            op_stack.push(t);
            break;

        case TokenType::R_PAR:
            while (!op_stack.empty() && op_stack.top().type != TokenType::L_PAR) {
                result.push_back(op_stack.top());
                op_stack.pop();
            }

            if (op_stack.empty())
                return {"too many closing brackets"};
            else
                op_stack.pop();

            // function with parens constitute a single entity like "sin(x)", so output function if there is one before L_PAR
            if (!op_stack.empty() && is_function(op_stack.top())) {
                result.push_back(op_stack.top());
                op_stack.pop();
            }
            break;

        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::MUL:
        case TokenType::DIV:
        case TokenType::POWER:
        case TokenType::F_NEG:
        case TokenType::F_ABS:
        case TokenType::F_SQRT:
        case TokenType::F_SIN:
        case TokenType::F_COS:
            while (!op_stack.empty()) {
                if (is_right_associative(t) || (priority(t) > priority(op_stack.top())))
                    break;
                result.push_back(op_stack.top());
                op_stack.pop();
            }
            op_stack.push(t);
            break;

        case TokenType::NUMBER:
        case TokenType::VARIABLE:
            result.push_back(t);
            break;

        default:
            return {"unknown token: " + t.value};
        }
    }

    while (!op_stack.empty()) {
        auto token = op_stack.top();
        if (token.type == TokenType::L_PAR)
            return {"missing closing bracket"};
        result.push_back(token);
        op_stack.pop();
    }

    return {std::move(result)};
}

string RpnBuilder::to_string(const vector<Token>& tokens) {
    string result {};
    for (const auto& t : tokens)
        result += t.value + " ";
    return std::move(result);
}

bool RpnBuilder::is_function(const Token& t) {
    switch (t.type) {
    case TokenType::F_NEG:
    case TokenType::F_ABS:
    case TokenType::F_SQRT:
    case TokenType::F_SIN:
    case TokenType::F_COS:
        return true;
    default:
        return false;
    }
}

int RpnBuilder::priority(const Token& t) {
    switch (t.type) {
    case TokenType::PLUS:
    case TokenType::MINUS:
        return 1;

    case TokenType::MUL:
    case TokenType::DIV:
        return 2;

    case TokenType::POWER:
        return 3;

    case TokenType::F_NEG:
    case TokenType::F_ABS:
    case TokenType::F_SQRT:
    case TokenType::F_SIN:
    case TokenType::F_COS:
        return 4;

    default:
        return 0;
    }
}

bool RpnBuilder::is_right_associative(const Token& t) {
    return (t.type == TokenType::POWER);
}

/**
 * @brief   take care of unary minus operator eg "-5" -> neg(5)
 */
void RpnBuilder::preprocess_unary_tokens(vector<Token>& tokens) {
    if (tokens.size() < 2)
        return;

    auto last = TokenType::INVALID;
    for (auto& t : tokens) {
        if (is_unary_minus(t.type, last))
            t = {TokenType::F_NEG, "neg"}; // replace unary "-" with "neg" to distinguish from binary "-"
        last = t.type;
    }
}

bool RpnBuilder::is_unary_minus(TokenType curr, TokenType prev) {
    // R_PAR, NUMBER and VARIABLE provide value, so in "VAL - sth" is not unary
    return curr == TokenType::MINUS && prev != TokenType::R_PAR && prev != TokenType::NUMBER && prev != TokenType::VARIABLE;
}

Optional<double> RpnEvaluator::eval(const vector<Token>& onp) {
    std::stack<double> value_stack;
    auto pop_value = [&value_stack](double& out) { if (value_stack.empty()) return false; out = value_stack.top(); value_stack.pop(); return true;};

    double left, right;
    double result = 0;
    for (Token op : onp) {
        switch (op.type) {
        case TokenType::NUMBER : {
            result = StringUtils::to_double(op.value);
            break;
        }

        case TokenType::VARIABLE : {
            if (auto var = get_value_for_name(op.value)) {
                result = var.value;
                break;
            }
            else
                return {"undefined identifier: " + op.value};
        }

        case TokenType::PLUS: {
            if (pop_value(right) && pop_value(left)) {
                result = left + right;
                break;
            }
            else
                return {"PLUS: missing argument"};
        }

        case TokenType::MINUS: {
            if (pop_value(right) && pop_value(left)) {
                result = left - right;
                break;
            }
            else
                return {"MINUS: missing argument"};
        }

        case TokenType::MUL: {
            if (pop_value(right) && pop_value(left)) {
                result = left * right;
                break;
            }
            else
                return {"MUL: missing argument"};
        }

        case TokenType::DIV: {
            if (pop_value(right) && pop_value(left)) {
                if (right == 0.0)
                    return {"DIV: division by zero"};

                result = left / right;
                break;
            }
            else
                return {"DIV: missing argument"};
        }

        case TokenType::POWER: {
            if (pop_value(right) && pop_value(left)) {
                if (left < 0.0 && (right != (long long)right))
                    return {"POWER: negative base to non integral power yields complex number"};
                result = math::pow(left, right);
                break;
            }
            else
                return {"POWER: missing argument"};
        }

        case TokenType::F_NEG: {
            if (pop_value(left)) {
                result = -left;
                break;
            }
            else
                return {"F_NEG: missing argument"};
        }

        case TokenType::F_ABS: {
            if (pop_value(left)) {
                result = (left > 0.0) ? left : -left;
                break;
            }
            else
                return {"F_ABS: missing argument"};
        }

        case TokenType::F_SQRT: {
            if (pop_value(left)) {
                if (left < 0.0)
                    return {"F_SQRT: negative sqrt argument yields complex number"};
                result = math::sqrt(left);
                break;
            }
            else
                return {"F_SQRT: missing argument"};
        }

        case TokenType::F_SIN: {
            if (pop_value(left)) {
                result = math::sin(left);
                break;
            }
            else
                return {"SIN: missing argument"};
        }

        case TokenType::F_COS: {
            if (pop_value(left)) {
                result = math::cos(left);
                break;
            }
            else
                return {"COS: missing argument"};
        }
        default: {
            return {"unknown token: " + op.value};
        }
        } // switch
        value_stack.push(result);
    } // for (Token op : onp)

    return {value_stack.top()};
}

Optional<double> RpnEvaluator::get_value_for_name(const string& name) {
    auto it = names.find(name);
    if (it != names.end())
        return {it->second};

    return {};
}

Calculator::Calculator() {
    define("pi", 3.141592653589793238462);
}

Optional<bool> Calculator::parse(const string& eq) {
    vector<Token> tokens;
    Tokenizer tokenizer;
    if (auto result = tokenizer.tokenize_equation(eq)) {
        tokens = std::move(result.value);
    }
    else {
        return {result.error_msg};
    }

    RpnBuilder builder;
    if (auto result = builder.build(tokens)) {
        onp = std::move(result.value);
    } else {
        return {result.error_msg};
    }

    return {true};
}

void Calculator::define(const string& name, double value) {
    evaluator.define(name, value);
}

Optional<double> Calculator::calc() {
    if (auto result = evaluator.eval(onp))
        return {result.value};
    else
        return {result.error_msg};

}
} /* namespace rpn */
} /* namespace ustd */
} /* namespace cstd */
