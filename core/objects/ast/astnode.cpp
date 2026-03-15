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

Int::Int(int val, int line) : ANode(NodeType::Int, line), value(val) {}

Float::Float(double val, int line) : ANode(NodeType::Float, line), value(val)
{
}

Bool::Bool(bool value, int line) : ANode(NodeType::Bool, line), value(value)
{
}

Variable::Variable(const std::string& name, int line) : ANode(NodeType::Variable, line),
    name(name)
{
}

Block::Block(QVector<pointer> &statements, int beginline, int endline): ANode(NodeType::Block), statements(statements),
    beginline(beginline), endline(endline)
{   
}

Assignment::Assignment(pointer left, pointer right, int line) : ANode(NodeType::Assignment, line), right(right), left(left)
{
}

Print::Print(pointer expression, int line) : ANode(NodeType::Print, line), expression(expression)
{
}

Unary::Unary(Lex::TokenType op, pointer value, int line) : ANode(NodeType::Unary, line), op(op), value(value)
{
}

String::String(const std::string &value, int line) : ANode(NodeType::String, line), value(value)
{
}

Call::Call(pointer caller, std::vector<pointer> &&params, int line) : ANode(NodeType::Call, line), caller(caller),
    params(params)
{
}

Index::Index(pointer obj, pointer expression, int line) : ANode(NodeType::Index, line), obj(obj), expression(expression)
{
}

Attribute::Attribute(pointer caller, const std::string &attributeName, int line) : ANode(NodeType::Attribute, line),
    caller(caller), attributeName(attributeName)
{
}

List::List(std::vector<pointer> &&elements, int line) : ANode(NodeType::List, line), elements(elements)
{

}

}
