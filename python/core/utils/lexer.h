#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <QObject>
#include <QString>
#include <QVector>
namespace Lex
{
    enum class TokenType {
        // 关键字
        IF, ELSE, ELIF, WHILE, PRINT, INPUT, TRUE, FALSE, IN, FOR,
        DEF, RETURN, CLASS, BREAK, CONTINUE,
        // 标识符 (变量名,函数名)
        IDENTIFIER,
        // 字面量
        INT, FLOAT,
        STRING,
        // 运算符
        PLUS, MINUS, STAR, SLASH, MOD,  // 算术
        EQ, NEQ, LT, GT, LTE, GTE,      // 比较
        AND, OR, NOT,                   // 逻辑
        // 赋值
        ASSIGN,
        // 括号
        LPAREN, RPAREN, LBRACKET, RBRACKET, LBRACE, RBRACE,
        // 冒号
        COLON,
        // 缩进
        INDENT, DEDENT,
        // 分隔符（逗号）
        COMMA, DOT,
        // 结束符
        EOF_TOKEN,
        // 错误标识符
        ERROR
    };
    struct Token {
        Token() {}
        TokenType type;
        std::string lexeme;  // 原始字符串
        int line;            // 行号（报错用）

        Token(TokenType t, std::string l, int ln) : type(t), lexeme(l), line(ln) {}

        // 辅助打印 Token
        friend std::ostream& operator<<(std::ostream& os, const Token& token) {
            os << "Token(";
            switch (token.type) {
            case TokenType::COLON: os << "COLON"; break;
            case TokenType::COMMA: os << "COMMA"; break;
            case TokenType::INDENT: os << "INDENT"; break;
            case TokenType::DEDENT: os << "DEDENT"; break;
            case TokenType::IF: os << "IF"; break;
            case TokenType::IN: os << "IN"; break;
            case TokenType::FOR: os << "FOR"; break;
            case TokenType::ELSE: os << "ELSE"; break;
            case TokenType::ELIF: os << "ELIF"; break;
            case TokenType::WHILE: os << "WHILE"; break;
            case TokenType::PRINT: os << "PRINT"; break;
            case TokenType::INPUT: os << "INPUT"; break;
            case TokenType::TRUE: os << "TRUE"; break;
            case TokenType::FALSE: os << "FALSE"; break;
            case TokenType::IDENTIFIER: os << "IDENTIFIER"; break;
            case TokenType::STRING: os << "STRING"; break;
            case TokenType::PLUS: os << "PLUS"; break;
            case TokenType::MINUS: os << "MINUS"; break;
            case TokenType::STAR: os << "STAR"; break;
            case TokenType::SLASH: os << "SLASH"; break;
            case TokenType::MOD: os << "MOD"; break;
            case TokenType::EQ: os << "EQ"; break;
            case TokenType::NEQ: os << "NEQ"; break;
            case TokenType::LT: os << "LT"; break;
            case TokenType::GT: os << "GT"; break;
            case TokenType::LTE: os << "LTE"; break;
            case TokenType::GTE: os << "GTE"; break;
            case TokenType::AND: os << "AND"; break;
            case TokenType::OR: os << "OR"; break;
            case TokenType::NOT: os << "NOT"; break;
            case TokenType::ASSIGN: os << "ASSIGN"; break;
            case TokenType::LPAREN: os << "LPAREN"; break;
            case TokenType::RPAREN: os << "RPAREN"; break;
            case TokenType::LBRACKET: os << "LBRACKET"; break;
            case TokenType::RBRACKET: os << "RBRACKET"; break;
            case TokenType::EOF_TOKEN: os << "EOF"; break;
            case TokenType::ERROR: os << "ERROR"; break;
            case TokenType::INT: os << "INT"; break;
            case TokenType::FLOAT: os << "FLOAT"; break;
            case TokenType::DEF: os << "DEF"; break;
            case TokenType::RETURN: os << "RETURN"; break;
            case TokenType::CLASS: os << "CLASS"; break;
            case TokenType::DOT: os << "DOT"; break;
            case TokenType::BREAK: os << "BREAK"; break;
            case TokenType::CONTINUE: os << "CONTINUE"; break;
            }
            os << ", '" << token.lexeme << "', line " << token.line << ")";
            return os;
        }
        static QString TypeToQStringStatic(TokenType type) {
            switch (type) {
                case TokenType::COLON:     return QString("COLON");
                case TokenType::COMMA:     return QString("COMMA");
                case TokenType::INDENT:    return QString("INDENT");
                case TokenType::DEDENT:    return QString("DEDENT");
                case TokenType::IF:        return QString("IF");
                case TokenType::IN:        return QString("IN");
                case TokenType::FOR:       return QString("FOR");
                case TokenType::ELSE:      return QString("ELSE");
                case TokenType::ELIF:      return QString("ELIF");
                case TokenType::WHILE:     return QString("WHILE");
                case TokenType::PRINT:     return QString("PRINT");
                case TokenType::INPUT:     return QString("INPUT");
                case TokenType::TRUE:      return QString("TRUE");
                case TokenType::FALSE:     return QString("FALSE");
                case TokenType::IDENTIFIER:return QString("IDENTIFIER");
                case TokenType::STRING:    return QString("STRING");
                case TokenType::PLUS:      return QString("PLUS");
                case TokenType::MINUS:     return QString("MINUS");
                case TokenType::STAR:      return QString("STAR");
                case TokenType::SLASH:     return QString("SLASH");
                case TokenType::MOD:       return QString("MOD");
                case TokenType::EQ:        return QString("EQ");
                case TokenType::NEQ:       return QString("NEQ");
                case TokenType::LT:        return QString("LT");
                case TokenType::GT:        return QString("GT");
                case TokenType::LTE:       return QString("LTE");
                case TokenType::GTE:       return QString("GTE");
                case TokenType::AND:       return QString("AND");
                case TokenType::OR:        return QString("OR");
                case TokenType::NOT:       return QString("NOT");
                case TokenType::ASSIGN:    return QString("ASSIGN");
                case TokenType::LPAREN:    return QString("LPAREN");
                case TokenType::RPAREN:    return QString("RPAREN");
                case TokenType::LBRACKET:  return QString("LBRACKET");
                case TokenType::RBRACKET:  return QString("RBRACKET");
                case TokenType::EOF_TOKEN: return QString("EOF");
                case TokenType::ERROR:     return QString("ERROR");
                case TokenType::INT:       return QString("INT");
                case TokenType::FLOAT:     return QString("FLOAT");
                case TokenType::DEF:       return QString("DEF");
                case TokenType::RETURN:    return QString("RETURN");
                case TokenType::CLASS:     return QString("CLASS");
                case TokenType::DOT:       return QString("DOT");
                case TokenType::BREAK:     return QString("BREAK");
                case TokenType::CONTINUE:  return QString("CONTINUE");
                default:                   return QString("UNKNOWN_TOKEN");
            }
        }
        QString TypeToQString(TokenType type) const {
            switch (type) {
                case TokenType::COLON:     return QString("COLON");
                case TokenType::COMMA:     return QString("COMMA");
                case TokenType::INDENT:    return QString("INDENT");
                case TokenType::DEDENT:    return QString("DEDENT");
                case TokenType::IF:        return QString("IF");
                case TokenType::IN:        return QString("IN");
                case TokenType::FOR:       return QString("FOR");
                case TokenType::ELSE:      return QString("ELSE");
                case TokenType::ELIF:      return QString("ELIF");
                case TokenType::WHILE:     return QString("WHILE");
                case TokenType::PRINT:     return QString("PRINT");
                case TokenType::INPUT:     return QString("INPUT");
                case TokenType::TRUE:      return QString("TRUE");
                case TokenType::FALSE:     return QString("FALSE");
                case TokenType::IDENTIFIER:return QString("IDENTIFIER");
                case TokenType::STRING:    return QString("STRING");
                case TokenType::PLUS:      return QString("PLUS");
                case TokenType::MINUS:     return QString("MINUS");
                case TokenType::STAR:      return QString("STAR");
                case TokenType::SLASH:     return QString("SLASH");
                case TokenType::MOD:       return QString("MOD");
                case TokenType::EQ:        return QString("EQ");
                case TokenType::NEQ:       return QString("NEQ");
                case TokenType::LT:        return QString("LT");
                case TokenType::GT:        return QString("GT");
                case TokenType::LTE:       return QString("LTE");
                case TokenType::GTE:       return QString("GTE");
                case TokenType::AND:       return QString("AND");
                case TokenType::OR:        return QString("OR");
                case TokenType::NOT:       return QString("NOT");
                case TokenType::ASSIGN:    return QString("ASSIGN");
                case TokenType::LPAREN:    return QString("LPAREN");
                case TokenType::RPAREN:    return QString("RPAREN");
                case TokenType::LBRACKET:  return QString("LBRACKET");
                case TokenType::RBRACKET:  return QString("RBRACKET");
                case TokenType::EOF_TOKEN: return QString("EOF");
                case TokenType::ERROR:     return QString("ERROR");
                case TokenType::INT:       return QString("INT");
                case TokenType::FLOAT:     return QString("FLOAT");
                case TokenType::DEF:       return QString("DEF");
                case TokenType::RETURN:    return QString("RETURN");
                case TokenType::CLASS:     return QString("CLASS");
                case TokenType::DOT:       return QString("DOT");
                case TokenType::BREAK:     return QString("BREAK");
                case TokenType::CONTINUE:  return QString("CONTINUE");
                default:                   return QString("UNKNOWN_TOKEN");
            }
        }
        QString TokenToQString(const Token& token) const {
            return QString("Token(%1, '%2', line %3)")
                    .arg(TypeToQString(token.type))
                    .arg(QString::fromStdString(token.lexeme))
                    .arg(token.line);
        }
    };

    const std::unordered_map<std::string, TokenType> KEYWORDS = {
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"elif", TokenType::ELIF},
        {"while", TokenType::WHILE},
        {"print", TokenType::PRINT},
        {"input", TokenType::INPUT},
        {"True", TokenType::TRUE},
        {"False", TokenType::FALSE},
        {"or", TokenType::OR},
        {"and", TokenType::AND},
        {"not", TokenType::NOT},
        {"in", TokenType::IN},
        {"for", TokenType::FOR},
        {"def", TokenType::DEF},
        {"return", TokenType::RETURN},
        {"class", TokenType::CLASS},
        {"break", TokenType::BREAK},
        {"continue", TokenType::CONTINUE},
    };
    // 我们拿到的程序是一大串由空格，缩进组成的文本，第一步是按照回车键分割字符串
    // 随后把每一行的字符串按照空格分隔，解析文本成token
    // 最后输出的是一个token流（就是一个token数组）
    class Lexer : public QObject
    {
        Q_OBJECT
    public:
        explicit Lexer(QObject* parent = nullptr);

        // 主要处理函数
        std::vector<Token> scanTokens(const std::string& text);

    private:
        // 按照回车分割函数
        std::vector<std::string> splitByLineBreak(const std::string& text);

        // 单行token解析函数
        std::vector<Token> scanPartToken(const std::string& line, int idx);

    private:
        std::vector<int> indentStack = { 0 };
        int bracketNesting = 0;
    signals:
        void finished(QVector<Token>);
    };
}





#endif // LEXER_H
