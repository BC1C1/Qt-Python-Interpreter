#include "astnode.h"

namespace Parse {

ANode::ANode(NodeType type, int line) : type(type), line(line)
{
}

NodeType ANode::getType() const
{
    return type;
}
ANode::~ANode()
{
}

int ANode::getLine() const
{
    return line;
}

Binary::Binary(TokenType op, pointer& left, pointer& right, int line) : ANode(NodeType::Binary, line),
     left(left), right(right), op(op)
{
}

QJsonObject Binary::toJson() const
{
    QJsonObject ret;
    ret["type"] = "binary";
    ret["op"] = Lex::Token::TypeToQStringStatic(this->op);
    ret["left"] = this->left->toJson();
    ret["right"] = this->right->toJson();
    ret["line"] = getLine();
    return ret;
}

Int::Int(int val, int line) : ANode(NodeType::Int, line), value(val) {}

QJsonObject Int::toJson() const
{
    QJsonObject ret;
    ret["type"] = "int";
    ret["value"] = this->value;
    ret["line"] = this->getLine();
    return ret;
}

Float::Float(double val, int line) : ANode(NodeType::Float, line), value(val)
{
}

QJsonObject Float::toJson() const
{
    QJsonObject ret;
    ret["type"] = "float";
    ret["value"] = this->value;
    ret["line"] = this->getLine();
    return ret;
}

Bool::Bool(bool value, int line) : ANode(NodeType::Bool, line), value(value)
{
}

QJsonObject Bool::toJson() const
{
    QJsonObject ret;
    ret["type"] = "bool";
    ret["value"] = this->value ? "true" : "false";
    ret["line"] = this->getLine();
    return ret;
}

Variable::Variable(const std::string& name, int line) : ANode(NodeType::Variable, line),
    name(name)
{
}

QJsonObject Variable::toJson() const
{
    QJsonObject ret;
    ret["type"] = "Variable";
    ret["value"] = QString::fromStdString(this->name);
    ret["line"] = this->getLine();
    return ret;
}

Block::Block(QVector<pointer> &statements, int beginline, int endline): ANode(NodeType::Block), statements(statements),
    beginline(beginline), endline(endline)
{   
}

QJsonObject Block::toJson() const
{
    QJsonObject ret;
    ret["type"] = "Block";
    QJsonArray array;
    for (const auto& s : statements) {
        array.append(s->toJson());
    }
    ret["statements"] = array;
    ret["beginline"] = this->beginline;
    ret["endline"] = this->endline;
    return ret;
}

Assignment::Assignment(pointer left, pointer right, int line) : ANode(NodeType::Assignment, line), right(right), left(left)
{
}

QJsonObject Assignment::toJson() const
{
    QJsonObject ret;
    ret["type"] = "assignment";
    ret["assignTo"] = this->left->toJson();
    ret["toAssign"] = this->right->toJson();
    ret["line"] = this->getLine();
    return ret;
}

Print::Print(pointer expression, int line) : ANode(NodeType::Print, line), expression(expression)
{
}

QJsonObject Print::toJson() const
{
    QJsonObject ret;
    ret["type"] = "print";
    ret["value"] = this->expression->toJson();
    ret["line"] = this->getLine();
    return ret;
}

Unary::Unary(Lex::TokenType op, pointer value, int line) : ANode(NodeType::Unary, line), op(op), value(value)
{
}

QJsonObject Unary::toJson() const
{
    QJsonObject ret;
    ret["type"] = "unary";
    ret["op"] = Lex::Token::TypeToQStringStatic(this->op);
    ret["value"] = this->value->toJson();
    ret["line"] = this->getLine();
    return ret;
}

String::String(const std::string &value, int line) : ANode(NodeType::String, line), value(value)
{
}

QJsonObject String::toJson() const
{
    QJsonObject ret;
    ret["type"] = "string";
    ret["value"] = QString::fromStdString(this->value);
    ret["line"] = this->getLine();
    return ret;
}

Call::Call(pointer caller, std::vector<pointer> &&params, int line) : ANode(NodeType::Call, line), caller(caller),
    params(params)
{
}

QJsonObject Call::toJson() const
{
    QJsonObject ret;
    ret["type"] = "call";
    ret["caller"] = this->caller->toJson();
    QJsonArray array;
    for (const auto& p : params) {
        array.append(p->toJson());
    }
    ret["params"] = array;
    ret["line"] = this->getLine();
    return ret;
}

Index::Index(pointer obj, pointer expression, int line) : ANode(NodeType::Index, line), obj(obj), expression(expression)
{
}

QJsonObject Index::toJson() const
{
    QJsonObject ret;
    ret["type"] = "index";
    ret["caller"] = this->obj->toJson();
    ret["expression"] = expression->toJson();
    ret["line"] = this->getLine();
    return ret;
}

Attribute::Attribute(pointer caller, const std::string &attributeName, int line) : ANode(NodeType::Attribute, line),
    caller(caller), attributeName(attributeName)
{
}

QJsonObject Attribute::toJson() const
{
    QJsonObject ret;
    ret["type"] = "Attribute";
    ret["caller"] = this->caller->toJson();
    ret["attributeName"] = QString::fromStdString(this->attributeName);
    ret["line"] = this->getLine();
    return ret;
}

List::List(std::vector<pointer> &&elements, int line) : ANode(NodeType::List, line), elements(elements)
{

}

QJsonObject List::toJson() const
{
    QJsonObject ret;
    ret["type"] = "list";
    QJsonArray array;
    for (const auto& ele : elements) {
        array.append(ele->toJson());
    }
    ret["elements"] = array;
    ret["line"] = this->getLine();
    return ret;
}

}
