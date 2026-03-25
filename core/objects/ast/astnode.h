#ifndef ASTNODE_H
#define ASTNODE_H
#include <QEnableSharedFromThis>
#include <QSharedPointer>
#include <QVector>
#include "core/utils/lexer.h"
#include <string>
#include <QJsonObject>
#include <QJsonArray>
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
    Attribute,
    Class,
    Break,
    Continue,
};

QString NodeTypeToQString(NodeType type);

class ANode : public QEnableSharedFromThis<ANode>
{
public:
    ANode(NodeType type, int line = 0);
    NodeType getType() const;
    virtual ~ANode();
    virtual QJsonObject toJson() const = 0;
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
    virtual QJsonObject toJson() const override;
};
class Int : public ANode
{
public:
    Int(int val, int line = 0);
    int value;
    virtual QJsonObject toJson() const override;
};

class Float : public ANode
{
public:
    Float(double val, int line = 0);
    double value;
    virtual QJsonObject toJson() const override;
};

class Bool : public ANode
{
public:
    Bool(bool value, int line = 0);
    bool value;
    virtual QJsonObject toJson() const override;
};

class Variable : public ANode
{
public:
    Variable(const std::string& name, int line = 0);
    std::string name;
    virtual QJsonObject toJson() const override;
};
class Block : public ANode
{
public:
    Block(QVector<pointer>& statements, int beginline, int endline, bool isNeedNewEnvir = false);
    QVector<pointer> statements;
    bool isNeedNewEnvir;
    int beginline;
    int endline;
    virtual QJsonObject toJson() const override;
};

class Assignment : public ANode {
public:
    Assignment(pointer left, pointer right, int line = 0);
    pointer right;
    pointer left;
    virtual QJsonObject toJson() const override;
};

class Print : public ANode {
public:
    Print(pointer expression, int line = 0);
    pointer expression;
    virtual QJsonObject toJson() const override;
};

class Unary : public ANode {
public:
    Unary(TokenType op, pointer value, int line = 0);
    TokenType op;
    pointer value;
    virtual QJsonObject toJson() const override;
};

class String : public ANode {
public:
    String(const std::string& value, int line = 0);
    std::string value;
    virtual QJsonObject toJson() const override;
};

class Call : public ANode {
public:
    Call(pointer caller, std::vector<pointer>&& params, int line = 0);
    pointer caller;
    std::vector<pointer> params;
    virtual QJsonObject toJson() const override;
};

class Index : public ANode {
public:
    Index(pointer obj, pointer expression, int line = 0);
    pointer obj;
    pointer expression;
    virtual QJsonObject toJson() const override;
};

class Attribute : public ANode {
public:
    Attribute(pointer caller, const std::string& attributeName, int line = 0);
    pointer caller;
    std::string attributeName;
    virtual QJsonObject toJson() const override;
};

class List : public ANode {
public:
    List(std::vector<pointer>&& elements, int line = 0);
    std::vector<pointer> elements;
    virtual QJsonObject toJson() const override;
};
class If: public ANode {
public:
    If(int line = 0, pointer block = nullptr, pointer condition = nullptr,
       QVector<pointer>&& elifs = QVector<pointer>(), pointer Else = nullptr);
    pointer condition;
    pointer block;
    QVector<pointer> elifs;
    pointer Else;
    virtual QJsonObject toJson() const override;
};

class While: public ANode {
public:
    While(pointer condition, pointer block, int line = 0);
    pointer condition;
    pointer block;
    virtual QJsonObject toJson() const override;
};

class Break: public ANode {
public:
    Break(int line = 0);
    virtual QJsonObject toJson() const override;
};

class Continue: public ANode {
public:
    Continue(int line = 0);
    virtual QJsonObject toJson() const override;
};

class For: public ANode {
public:
    For(pointer loopVar, pointer listObj, pointer block, int line);
    pointer loopVar;
    pointer listObj;
    pointer block;
    virtual QJsonObject toJson() const override;
};
}


#endif // ASTNODE_H
