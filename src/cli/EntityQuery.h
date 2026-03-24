#pragma once
#include "Entity.h"
#include <string>
#include <vector>
#include <memory>

class QueryNode {
public:
    virtual ~QueryNode() = default;
    virtual bool evaluate(Entity* ent) = 0;
};

class EntityQuery {
public:
    EntityQuery(const std::string& queryString);
    bool evaluate(Entity* ent);
    bool isValid() const { return root != nullptr; }

private:
    std::unique_ptr<QueryNode> root;
    std::string error;

    struct Token {
        enum Type {
            KEYVALUE, // Key or Value
            AND,
            OR,
            NOT,
            EQUALS,
            NOT_EQUALS,
            LPAREN,
            RPAREN,
            END
        } type;
        std::string value;
    };

    std::vector<Token> tokens;
    size_t currentToken;

    void tokenize(const std::string& q);
    std::unique_ptr<QueryNode> parseExpression();
    std::unique_ptr<QueryNode> parseTerm();
    std::unique_ptr<QueryNode> parseFactor();
    std::unique_ptr<QueryNode> parsePrimary();
};
