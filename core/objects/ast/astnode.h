#ifndef ASTNODE_H
#define ASTNODE_H
#include <QEnableSharedFromThis>
#include <QSharedPointer>
#include <QVector>
#include "core/utils/lexer.h"
#include <string>
namespace Parse {
enum class NodeType {
    Program,
    Block,
    Assignment,
    If,
    Elif,
    Else,
    While,
    For,
    Binary,
    Unary,
    Variable,
    Number, Int, Float,
    String,
    List,
    Index,
    Bool,
    Print,
    FunctionDefine,
    Return,
    Call,
    Attribute, Class
};

class ANode : public QEnableSharedFromThis<ANode>
{
public:
    ANode(NodeType type, int line = 0);
    NodeType getType() const;
    virtual ~ANode();
    int getLine() const;
private:
    NodeType type;
    int line;
};
using pointer = QSharedPointer<ANode>;

using Lex::TokenType;
class Binary : public ANode
{
public:
    Binary(TokenType op, pointer& left, pointer& right, int line = 0);
    pointer left;
    pointer right;
    TokenType op;
};
class Int : public ANode
{
public:
    Int(int val, int line = 0);
    int value;
};

class Float : public ANode
{
public:
    Float(double val, int line = 0);
    double value;
};

class Bool : public ANode
{
public:
    Bool(bool value, int line = 0);
    bool value;
};

class Variable : public ANode
{
public:
    Variable(const std::string& name, int line = 0);
    std::string name;
};
class Block : public ANode
{
public:
    Block(QVector<pointer>& statements, int beginline, int endline);
    QVector<pointer> statements;
    int beginline;
    int endline;
};

class Assignment : public ANode {
public:
    Assignment(pointer left, pointer right, int line = 0);
    pointer right;
    pointer left;
};

class Print : public ANode {
public:
    Print(pointer expression, int line = 0);
    pointer expression;
};

class Unary : public ANode {
public:
    Unary(TokenType op, pointer value, int line = 0);
    TokenType op;
    pointer value;
};

class String : public ANode {
public:
    String(const std::string& value, int line = 0);
    std::string value;
};

class Call : public ANode {
public:
    Call(pointer caller, std::vector<pointer>&& params, int line = 0);
    pointer caller;
    std::vector<pointer> params;
};

class Index : public ANode {
public:
    Index(pointer obj, pointer expression, int line = 0);
    pointer obj;
    pointer expression;
};

class Attribute : public ANode {
public:
    Attribute(pointer caller, const std::string& attributeName, int line = 0);
    pointer caller;
    std::string attributeName;
};

class List : public ANode {
public:
    List(std::vector<pointer>&& elements, int line = 0);
    std::vector<pointer> elements;
};
}


#endif // ASTNODE_H
