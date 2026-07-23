#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include "core/objects/ast/astnode.h"
#include "core/utils/lexer.h"
#include "core/utils/functions.h"
#include "Exception.h"

namespace Parse {
using Lex::Token;
using std::string;
class Parser : public QObject
{
    Q_OBJECT
public:
    explicit Parser(QObject *parent = nullptr);
    pointer parse(const std::vector<Token>& tokens);
    void clearAll();
private:
    pointer parseBlock(bool isNeedNewEnvir);

    pointer parseAssign();
    pointer parsePrint();
    pointer parseStatement();

    pointer parseLValue();

    pointer parseIfStmt();
    pointer parseElifStmt();
    pointer parseElseStmt();

    pointer parseWhile();
    pointer parseFor();

    pointer parseBreak();
    pointer parseContinue();

    pointer parseFunction();
    pointer parseReturn();

    pointer parseClass();

    pointer parseImport();

    pointer parseGlobal();
private:
    pointer parseExpression();
    pointer parseOr();
    pointer parseAnd();
    pointer parseIn();
    pointer parseComparison();
    pointer parseTerm();
    pointer parseFactor();
    pointer parseUnary();
    pointer parsePrimary();
    pointer parseList();
    pointer parseDict();
private:
    void throwErrorMsg(string text);
private:
    bool isAtEnd();

    const Token& peek();

    Token peekNext();

    const Token& consume(TokenType type, const std::string& errorMsg);

    bool check(TokenType type);

    bool match(TokenType type);

    const std::vector<Token>& getTokens() const;

    bool outOfRange(int bais = 0) const;

    // no error check
    int getInstantLine();

private:
    const std::vector<Token>* tokens;
    pointer resultAst;
    size_t current;
    bool isParsing;
signals:

};

}


#endif // PARSER_H
