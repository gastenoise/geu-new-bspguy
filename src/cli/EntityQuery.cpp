#include "EntityQuery.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

class ComparisonNode : public QueryNode {
public:
    std::string key;
    std::string value;
    bool isNot;

    ComparisonNode(const std::string& k, const std::string& v, bool n) : key(k), value(v), isNot(n) {}

    bool evaluate(Entity* ent) override {
        std::string entValue = "";
        if (ent->keyvalues.count(key)) {
            entValue = ent->keyvalues.at(key);
        }

        bool match = matchWildcard(value, entValue);
        return isNot ? !match : match;
    }

private:
    bool matchWildcard(const std::string& pattern, const std::string& text) {
        if (pattern == "*") return true;
        if (pattern.empty()) return text.empty();

        size_t n = text.size();
        size_t m = pattern.size();

        // Optimized wildcard matching using two rows (prev and curr) to save space
        std::vector<bool> dp(m + 1, false);
        dp[0] = true;

        for (size_t j = 1; j <= m; j++) {
            if (pattern[j - 1] == '*') {
                dp[j] = dp[j - 1];
            }
        }

        for (size_t i = 1; i <= n; i++) {
            bool prev_diag = dp[0];
            dp[0] = false;
            for (size_t j = 1; j <= m; j++) {
                bool next_prev_diag = dp[j];
                if (pattern[j - 1] == '*') {
                    dp[j] = dp[j] || dp[j - 1];
                } else if (pattern[j - 1] == text[i - 1]) {
                    dp[j] = prev_diag;
                } else {
                    dp[j] = false;
                }
                prev_diag = next_prev_diag;
            }
        }

        return dp[m];
    }
};

class LogicalNode : public QueryNode {
public:
    enum Op { AND, OR } op;
    std::unique_ptr<QueryNode> left;
    std::unique_ptr<QueryNode> right;

    LogicalNode(Op o, std::unique_ptr<QueryNode> l, std::unique_ptr<QueryNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}

    bool evaluate(Entity* ent) override {
        if (op == AND) {
            return left->evaluate(ent) && right->evaluate(ent);
        } else {
            return left->evaluate(ent) || right->evaluate(ent);
        }
    }
};

class NotNode : public QueryNode {
public:
    std::unique_ptr<QueryNode> child;

    NotNode(std::unique_ptr<QueryNode> c) : child(std::move(c)) {}

    bool evaluate(Entity* ent) override {
        return !child->evaluate(ent);
    }
};

EntityQuery::EntityQuery(const std::string& queryString) {
    tokenize(queryString);
    currentToken = 0;
    if (!tokens.empty()) {
        root = parseExpression();
    }
}

bool EntityQuery::evaluate(Entity* ent) {
    if (root) {
        return root->evaluate(ent);
    }
    return false;
}

void EntityQuery::tokenize(const std::string& q) {
    for (size_t i = 0; i < q.size(); ) {
        if (isspace((unsigned char)q[i])) {
            i++;
            continue;
        }

        size_t start_i = i;
        if (q[i] == '(') {
            tokens.push_back({ Token::LPAREN, "(" });
            i++;
        } else if (q[i] == ')') {
            tokens.push_back({ Token::RPAREN, ")" });
            i++;
        } else if (q[i] == '=') {
            if (i + 1 < q.size() && q[i + 1] == '=') {
                tokens.push_back({ Token::EQUALS, "==" });
                i += 2;
            } else {
                tokens.push_back({ Token::EQUALS, "=" });
                i++;
            }
        } else if (q[i] == '!') {
            if (i + 1 < q.size() && q[i + 1] == '=') {
                tokens.push_back({ Token::NOT_EQUALS, "!=" });
                i += 2;
            } else {
                tokens.push_back({ Token::NOT, "!" });
                i++;
            }
        } else if (q[i] == '"') {
            std::string s;
            i++;
            while (i < q.size() && q[i] != '"') {
                if (q[i] == '\\' && i + 1 < q.size()) {
                    i++;
                }
                s += q[i++];
            }
            if (i < q.size()) i++;
            tokens.push_back({ Token::KEYVALUE, s });
        } else {
            std::string s;
            while (i < q.size() && !isspace((unsigned char)q[i]) && q[i] != '(' && q[i] != ')' && q[i] != '=' && q[i] != '!' && q[i] != '"') {
                s += q[i++];
            }

            if (!s.empty()) {
                std::string upperS = s;
                std::transform(upperS.begin(), upperS.end(), upperS.begin(), [](unsigned char c) { return (unsigned char)std::toupper(c); });

                if (upperS == "AND") tokens.push_back({ Token::AND, s });
                else if (upperS == "OR") tokens.push_back({ Token::OR, s });
                else if (upperS == "NOT") tokens.push_back({ Token::NOT, s });
                else tokens.push_back({ Token::KEYVALUE, s });
            }
        }

        if (i == start_i) {
            i++; // Ensure we always advance to avoid infinite loop
        }
    }
    tokens.push_back({ Token::END, "" });
}

std::unique_ptr<QueryNode> EntityQuery::parseExpression() {
    auto node = parseTerm();
    if (!node) return nullptr;
    while (tokens[currentToken].type == Token::OR) {
        currentToken++;
        auto right = parseTerm();
        if (right) {
            node = std::make_unique<LogicalNode>(LogicalNode::OR, std::move(node), std::move(right));
        } else {
            break;
        }
    }
    return node;
}

std::unique_ptr<QueryNode> EntityQuery::parseTerm() {
    auto node = parseFactor();
    if (!node) return nullptr;
    while (tokens[currentToken].type == Token::AND) {
        currentToken++;
        auto right = parseFactor();
        if (right) {
            node = std::make_unique<LogicalNode>(LogicalNode::AND, std::move(node), std::move(right));
        } else {
            break;
        }
    }
    return node;
}

std::unique_ptr<QueryNode> EntityQuery::parseFactor() {
    if (tokens[currentToken].type == Token::NOT) {
        currentToken++;
        auto child = parseFactor();
        if (child) {
            return std::make_unique<NotNode>(std::move(child));
        }
        return nullptr;
    }
    return parsePrimary();
}

std::unique_ptr<QueryNode> EntityQuery::parsePrimary() {
    if (tokens[currentToken].type == Token::LPAREN) {
        currentToken++;
        auto node = parseExpression();
        if (tokens[currentToken].type == Token::RPAREN) {
            currentToken++;
        }
        return node;
    } else if (tokens[currentToken].type == Token::KEYVALUE) {
        std::string key = tokens[currentToken].value;
        currentToken++;
        if (tokens[currentToken].type == Token::EQUALS || tokens[currentToken].type == Token::NOT_EQUALS) {
            bool isNot = tokens[currentToken].type == Token::NOT_EQUALS;
            currentToken++;
            if (tokens[currentToken].type == Token::KEYVALUE) {
                std::string value = tokens[currentToken].value;
                currentToken++;
                return std::make_unique<ComparisonNode>(key, value, isNot);
            } else {
                // Allow matching empty string: key= or key!= followed by non-KEYVALUE
                return std::make_unique<ComparisonNode>(key, "", isNot);
            }
        }
    }
    return nullptr;
}
