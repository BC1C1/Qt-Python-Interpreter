#include "parser.h"
namespace Parse {

Parser::Parser(QObject *parent) : QObject(parent), resultAst(nullptr), current(0), isParsing(false)
{
}

pointer Parser::parse(const std::vector<Lex::Token> &tokens)
{
    isParsing = true;
    this->tokens = &tokens;
    QVector<pointer> statements;
    while (!isAtEnd()) {
        statements.push_back(parseStatement());
    }
    int beginline = 0;
    int endline = 0;
    if (!statements.isEmpty()) {
        beginline = statements[0]->getLine();
        endline = statements[statements.size() - 1]->getLine();
    }
    auto ret = makeShared<Block>(statements, beginline, endline);
    isParsing = false;
    this->resultAst = ret;
    return ret;
}

pointer Parser::parseBlock(bool isNeedNewEnvir)
{
    consume(TokenType::INDENT, "需要缩进");
    auto beginline = 0;
    auto endline = 0;
    QVector<pointer> statements;
    while (!isAtEnd() && !check(TokenType::DEDENT)) {
        statements.push_back(parseStatement());
    }
    if (!statements.isEmpty()) {
        beginline = statements[0]->getLine();
        endline = statements[statements.size() - 1]->getLine();
    }
    consume(TokenType::DEDENT, "缺少缩进");

    return makeShared<Block>(statements, beginline, endline, isNeedNewEnvir);
}


pointer Parser::parseAssign()
{
    size_t curr = current;
    auto left = parseLValue();
    if (!left) {
        current = curr;
        return nullptr;
    }
    if (!match(TokenType::ASSIGN)) {
        current = curr;
        return nullptr;
    }
    auto right = parseExpression();
    return makeShared<Assignment>(left, right, getInstantLine());
}

pointer Parser::parsePrint()
{
    if (!match(TokenType::PRINT)) return nullptr;
    consume(TokenType::LPAREN, u8"缺失的左括号");
    auto expression = parseExpression();
    consume(TokenType::RPAREN, u8"缺失的右括号");
    return makeShared<Print>(expression, getInstantLine());
}

pointer Parser::parseStatement()
{
    auto assignStmt = parseAssign();
    if (assignStmt) return assignStmt;
    auto printStmt = parsePrint();
    if (printStmt) return printStmt;
    auto ifStmt = parseIfStmt();
    if (ifStmt) return ifStmt;
    auto whileStmt = parseWhile();
    if (whileStmt) return whileStmt;
    auto forStmt = parseFor();
    if (forStmt) return forStmt;
    auto breakStmt = parseBreak();
    if (breakStmt) return breakStmt;
    auto continueStmt = parseContinue();
    if (continueStmt) return continueStmt;
    return parseExpression();
}

pointer Parser::parseLValue()
{
    if (!match(TokenType::IDENTIFIER)) return nullptr;
    pointer ret = makeShared<Variable>(getTokens()[current - 1].lexeme, getInstantLine());
    while (check(TokenType::LBRACKET) || check(TokenType::LPAREN) || check(TokenType::DOT)) {
        auto type = getTokens()[current].type;
        current++;
        switch (type)
        {
        case Lex::TokenType::LPAREN: {
            std::vector<pointer> expressions;
            while (!check(TokenType::RPAREN)) {
                auto ele = parseExpression();
                if (ele)
                    expressions.push_back(ele);
                if (match(TokenType::COMMA)) {
                    continue;
                }
                else if (!check(TokenType::RPAREN)) {
                    throwErrorMsg(u8"元素之间缺少逗号");
                }
            }
            consume(TokenType::RPAREN, u8"缺少右括号");
            ret = makeShared<Parse::Call>(ret, std::move(expressions), getInstantLine());
            break;
        }
        case Lex::TokenType::LBRACKET: {
            auto indexExpr = parseExpression();
            consume(TokenType::RBRACKET, u8"索引缺少右分隔符 ']'");
            ret = makeShared<Parse::Index>(ret, indexExpr, getInstantLine());
            break;
        }
        case Lex::TokenType::DOT: {
            auto& attribute = consume(TokenType::IDENTIFIER, u8"无效的属性");
            ret = makeShared<Parse::Attribute>(ret, attribute.lexeme, getInstantLine());
            break;
        }
        default: break; // 无意义，仅为了消除警告
        }
    }
    return ret;
}

pointer Parser::parseIfStmt()
{
    if (!match(TokenType::IF)) return nullptr;
    auto line = getInstantLine();
    auto condition = parseExpression();
    if (!condition)  {
        QString errMsg = QString(u8"If语句缺少条件, 行号: %1").arg(line);
        throwErrorMsg(errMsg.toUtf8().data());
    }
    consume(TokenType::COLON, u8"if语句缺少冒号");
    auto block = parseBlock(false);
    QVector<pointer> elifs;
    while (check(TokenType::ELIF)) {
        auto elifstmt = parseElifStmt();
        if (elifstmt)
            elifs.push_back(elifstmt);
    }
    auto Else = parseElseStmt();

    return makeShared<If>(line, block, condition, std::move(elifs), Else);
}

pointer Parser::parseElifStmt()
{
    if (!match(TokenType::ELIF)) return nullptr;
    auto line = getInstantLine();
    auto condition = parseExpression();
    if (!condition)  {
        QString errMsg = QString(u8"Elif语句缺少条件, 行号: %1").arg(line);
        throwErrorMsg(errMsg.toUtf8().data());
    }
    consume(TokenType::COLON, u8"elif子句缺少冒号");
    auto block = parseBlock(false);
    return makeShared<If>(line, block, condition);
}

pointer Parser::parseElseStmt()
{
    if (!match(TokenType::ELSE)) return nullptr;
    auto line = getInstantLine();
    consume(TokenType::COLON, u8"else子句缺少冒号");
    auto block = parseBlock(false);
    return makeShared<If>(line, block);
}

pointer Parser::parseWhile()
{
    if (!match(TokenType::WHILE)) return nullptr;
    auto line = getInstantLine();
    auto condition = parseExpression();
    if (!condition)
        throwErrorMsg(QString(u8"while缺少条件, 行: %1").arg(line).toUtf8().data());
    consume(TokenType::COLON, QString(u8"缺少的冒号, 行: %1").arg(line).toUtf8().data());
    auto block = parseBlock(false);
    if (!block)
        throwErrorMsg(QString(u8"空语句块, 行: %1").arg(line).toUtf8().data());
    return makeShared<While>(condition, block, line);
}

pointer Parser::parseFor()
{
    if (!match(TokenType::FOR)) return nullptr;
    auto line = getInstantLine();
    auto loopVar = parseLValue();
    consume(TokenType::IN, u8"for语句需要in");
    auto list = parseExpression();
    consume(TokenType::COLON, u8"for语句缺少冒号");
    auto block = parseBlock(false);
    return makeShared<For>(loopVar, list, block, line);
}

pointer Parser::parseBreak()
{
    if (!match(TokenType::BREAK)) return nullptr;
    auto line = getInstantLine();
    return makeShared<Break>(line);
}

pointer Parser::parseContinue()
{
    if (!match(TokenType::CONTINUE)) return nullptr;
    auto line = getInstantLine();
    return makeShared<Continue>(line);
}

pointer Parser::parseExpression()
{
    return parseOr();
}

pointer Parser::parseOr()
{
    auto left = parseAnd();
    while (!isAtEnd() && match(TokenType::OR)) {
        auto right = parseAnd();
        left = makeShared<Binary>(TokenType::OR, left, right, getInstantLine());
    }
    return left;
}

pointer Parser::parseAnd()
{
    auto left = parseIn();
    while (!isAtEnd() && match(TokenType::AND)) {
        auto right = parseIn();
        left = makeShared<Binary>(TokenType::AND, left, right, getInstantLine());
    }
    return left;
}

pointer Parser::parseIn()
{
    auto left = parseComparison();

    while (!isAtEnd() && match(TokenType::IN)) {
        auto right = parseComparison();
        left = makeShared<Binary>(TokenType::IN, left, right, getInstantLine());
    }
    return left;
}

pointer Parser::parseComparison()
{
    auto left = parseTerm();

    while (!isAtEnd() && (
        check(TokenType::EQ) || check(TokenType::NEQ) ||
        check(TokenType::LT) || check(TokenType::LTE) ||
        check(TokenType::GT) || check(TokenType::GTE)
        )) {
        auto opTokenType = getTokens()[current].type;
        current++; // 上面用的是check所以要加一
        auto right = parseTerm();
        left = makeShared<Binary>(opTokenType, left, right, getInstantLine());
    }
    return left;
}

pointer Parser::parseTerm()
{
    auto left = parseFactor();

    while (!isAtEnd() && (check(TokenType::PLUS) || check(TokenType::MINUS))) {
        auto opTokenType = getTokens()[current].type;
        current++;
        auto right = parseFactor();
        left = makeShared<Binary>(opTokenType, left, right, getInstantLine());
    }
    return left;
}

pointer Parser::parseFactor()
{
    auto left = parseUnary();

    while (!isAtEnd() && (
        check(TokenType::STAR) || check(TokenType::SLASH) ||
        check(TokenType::MOD)
        )) {
        auto opTokenType = getTokens()[current].type;
        current++;
        auto right = parseUnary();
        left = makeShared<Binary>(opTokenType, left, right, getInstantLine());
    }
    return left;
}

pointer Parser::parseUnary()
{
    if (check(TokenType::MINUS) || check(TokenType::NOT)) {
        auto opType = getTokens()[current].type;
        current++;
        pointer inner = parseUnary();
        return makeShared<Unary>(opType, inner, getInstantLine());
    }
    return parsePrimary();
}

pointer Parser::parsePrimary()
{
    // 最小元素
    auto token = getTokens()[current];
    auto type = token.type;
    pointer ret = nullptr;
    switch (type)
    {
    case Lex::TokenType::STRING: {
        current += 1;
        ret = makeShared<String>(token.lexeme, getInstantLine());
        break;
    }
    case Lex::TokenType::INT: {
        current += 1;
        int value = std::stoi(token.lexeme);
        ret = makeShared<Int>(value, getInstantLine());
        break;
    }
    case Lex::TokenType::FLOAT: {
        current += 1;
        double value = std::stod(token.lexeme);
        ret = makeShared<Float>(value, getInstantLine());
        break;
    }
    case Lex::TokenType::TRUE: {
        current += 1;
        ret = makeShared<Bool>(true, getInstantLine());
        break;
    }
    case Lex::TokenType::FALSE: {
        current += 1;
        ret = makeShared<Bool>(false, getInstantLine());
        break;
    }
    case Lex::TokenType::IDENTIFIER: {
        current += 1;
        ret = makeShared<Variable>(token.lexeme, getInstantLine());
        break;
    }
    case Lex::TokenType::LPAREN: {
        // 开始匹配
        consume(TokenType::LPAREN, "u8缺失的左括号");
        auto inner = parseExpression();
        consume(TokenType::RPAREN, "u8缺失的右括号");
        ret = inner;
        break;
        // return之前就consume了右括号了，这个语句只匹配了一层括号
    }
    case Lex::TokenType::LBRACKET: {
        auto inner = parseList();
        ret = inner;
        break;
    }
    case Lex::TokenType::COMMA: {
        break;
    }
    case Lex::TokenType::RPAREN: {
        // 直接提交错误
        throwErrorMsg("缺失的左括号");
        current += 1;
        break;
    }
    case Lex::TokenType::EOF_TOKEN: {
        throwErrorMsg(u8"不完整的表达式");
        current += 1;
        break;
    }
    case Lex::TokenType::ERROR: {
        throwErrorMsg(u8"错误的Token");
        current += 1;
        break;
    }
    default:
        throwErrorMsg(u8"未知的符号");
        current += 1;
        break;
    }

    while (check(TokenType::LBRACKET) || check(TokenType::LPAREN) || check(TokenType::DOT)) {
        auto type = getTokens()[current].type;
        current++;
        switch (type)
        {
        case Lex::TokenType::LPAREN: {
            std::vector<pointer> expressions;
            while (!check(TokenType::RPAREN)) {
                auto ele = parseExpression();
                if (ele)
                    expressions.push_back(ele);
                if (match(TokenType::COMMA)) {
                    continue;
                }
                else if (!check(TokenType::RPAREN)) {
                    throwErrorMsg(u8"元素之间缺少逗号");
                }
            }
            consume(TokenType::RPAREN, u8"缺少右括号");
            ret = makeShared<Parse::Call>(ret, std::move(expressions), getInstantLine());
            break;
        }
        case Lex::TokenType::LBRACKET: {
            auto indexExpr = parseExpression();
            consume(TokenType::RBRACKET, u8"索引缺少右分隔符 ']'");
            ret = makeShared<Parse::Index>(ret, indexExpr, getInstantLine());
            break;
        }
        case Lex::TokenType::DOT: {
            auto& attribute = consume(TokenType::IDENTIFIER, u8"无效的属性");
            ret = makeShared<Parse::Attribute>(ret, attribute.lexeme, getInstantLine());
            break;
        }
        default: break; // 无意义，仅为了消除警告
        }
    }

    return ret;
}

pointer Parser::parseList()
{
    consume(TokenType::LBRACKET, u8"缺失的左中括号");
    std::vector<pointer> elements;

    do {
        if (check(TokenType::RBRACKET)) break;

        auto ele = parseExpression();
        if (ele) elements.push_back(ele);

        if (!check(TokenType::RBRACKET) && !match(TokenType::COMMA)) {
            throwErrorMsg(u8"列表元素之间缺少逗号");
            break;
        }
    } while (true);

    consume(TokenType::RBRACKET, u8"缺失的右中括号");
    return makeShared<List>(std::move(elements), getInstantLine());
}

void Parser::throwErrorMsg(std::string text)
{
    throw std::runtime_error(text);
}

bool Parser::isAtEnd()
{
    return outOfRange() || (getTokens()[current].type == TokenType::EOF_TOKEN);
}

const Lex::Token &Parser::peek()
{
    return getTokens()[current];
}

Lex::Token Parser::peekNext()
{
    if (!outOfRange(1))
        return getTokens()[current + 1];
    return Token(TokenType::ERROR, "out_of_range", getTokens()[current].line + 1);
}

const Lex::Token &Parser::consume(Lex::TokenType type, const std::string &errorMsg)
{
    if (!match(type)) {
        throwErrorMsg(errorMsg);
    }
    return getTokens()[current - 1];
}

bool Parser::check(Lex::TokenType type)
{
    if (outOfRange()) return false;
    return getTokens()[current].type == type;
}

bool Parser::match(Lex::TokenType type)
{
    if (check(type)){
        current++;
        return true;
    }
    return false;
}

const std::vector<Lex::Token> &Parser::getTokens() const
{
    return *tokens;
}

bool Parser::outOfRange(int bias) const
{
    return (current + bias) >= getTokens().size() || (current + bias) < 0;
}

int Parser::getInstantLine()
{
    return getTokens()[current - 1].line;
}

}

