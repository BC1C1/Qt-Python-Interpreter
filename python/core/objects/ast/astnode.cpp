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

Variable::Variable(const std::string& name, bool isSuper, int line)
    : ANode(NodeType::Variable, line),
    name(name), isSuper(isSuper)
{
}

QJsonObject Variable::toJson() const
{
    QJsonObject ret;
    ret["type"] = "Variable";
    ret["value"] = QString::fromStdString(this->name);
    ret["line"] = this->getLine();
    ret["isSuper"] = isSuper ? "true" : "false";
    return ret;
}

Block::Block(const QVector<pointer> &statements, int beginline, int endline, bool isneed)
    : ANode(NodeType::Block), statements(statements), isNeedNewEnvir(isneed),
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
    ret["isNeedNewEnvironment"] = this->isNeedNewEnvir;
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

String::String(const std::string &value, int line) 
    : ANode(NodeType::String, line), value(value)
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

QString NodeTypeToQString(NodeType type)
{
    // 使用 switch 语句匹配每个枚举值，返回对应的字符串
    switch (type) {
        case NodeType::Program:        return QString("Program");
        case NodeType::Block:          return QString("Block");
        case NodeType::Assignment:     return QString("Assignment");
        case NodeType::If:             return QString("If");
        case NodeType::Elif:           return QString("Elif");
        case NodeType::Else:           return QString("Else");
        case NodeType::While:          return QString("While");
        case NodeType::For:            return QString("For");
        case NodeType::Binary:         return QString("Binary");
        case NodeType::Unary:          return QString("Unary");
        case NodeType::Variable:       return QString("Variable");
        case NodeType::Number:         return QString("Number");
        case NodeType::Int:            return QString("Int");
        case NodeType::Float:          return QString("Float");
        case NodeType::String:         return QString("String");
        case NodeType::List:           return QString("List");
        case NodeType::Index:          return QString("Index");
        case NodeType::Bool:           return QString("Bool");
        case NodeType::Print:          return QString("Print");
        case NodeType::FunctionDefine: return QString("FunctionDefine");
        case NodeType::Return:         return QString("Return");
        case NodeType::Call:           return QString("Call");
        case NodeType::Attribute:      return QString("Attribute");
        case NodeType::Class:          return QString("Class");
        case NodeType::Break:          return QString("Break");
        case NodeType::Continue:       return QString("Continue");
        default:                       return QString("UnknownNodeType");
    }
}

If::If(int line, pointer block, pointer condition, QVector<pointer> &&elifs, pointer Else)
: ANode(NodeType::If, line), condition(condition), block(block), elifs(elifs), Else(Else)
{

}

QJsonObject If::toJson() const
{
    QJsonObject ret;
    ret["type"] = "If";
    ret["line"] = getLine();
    if (condition)
        ret["condition"] = condition->toJson();
    if (block)
        ret["ifTrueExecute"] = block->toJson();
    QJsonArray array;
    for (const auto& e : elifs) {
        if (e)
            array.append(e->toJson());
    }
    ret["elifs"] = array;
    if (Else)
        ret["else"] = Else->toJson();

    return ret;
}

While::While(pointer condition, pointer block, int line) : ANode(NodeType::While, line),
    condition(condition), block(block)
{
}

QJsonObject While::toJson() const
{
    QJsonObject ret;
    ret["type"] = "While";
    ret["condition"] = condition->toJson();
    ret["toExecute"] = block->toJson();
    ret["line"] = getLine();
    return ret;
}

Break::Break(int line): ANode(NodeType::Break, line)
{
}

QJsonObject Break::toJson() const
{
    QJsonObject ret;
    ret["type"] = "Break";
    ret["line"] = getLine();
    return ret;
}

Continue::Continue(int line): ANode(NodeType::Continue, line)
{

}

QJsonObject Continue::toJson() const
{
    QJsonObject ret;
    ret["type"] = "Continue";
    ret["line"] = getLine();
    return ret;
}

For::For(pointer loopVar, pointer listObj, pointer block, int line): ANode(NodeType::For, line),
   loopVar(loopVar), listObj(listObj), block(block)
{

}

QJsonObject For::toJson() const
{
    QJsonObject ret;
    ret["type"] = "For";
    ret["line"] = getLine();
    ret["iterVar"] = loopVar->toJson();
    ret["list"] = listObj->toJson();
    ret["block"] = block->toJson();
    return ret;
}

Function::Function(int line, const std::string& functionName, pointer listParams, pointer block)
    : ANode(NodeType::FunctionDefine, line), functionName(functionName), listParams(listParams), block(block)
{

}

QJsonObject Function::toJson() const
{
    QJsonObject ret;
    ret["type"] = "Function";
    ret["params"] = listParams->toJson();
    ret["functionNamee"] = QString::fromStdString(functionName);
    ret["block"] = block->toJson();
    ret["line"] = getLine();
    return ret;
}

Return::Return(pointer expression, int line): ANode(NodeType::Return, line), expression(expression)
{

}

QJsonObject Return::toJson() const
{
    QJsonObject ret;
    ret["type"] = "Return";
    ret["toReturn"] = expression->toJson();
    ret["line"] = getLine();
    return ret;
}

Class::Class(
    QVector<pointer>& functions, 
    QString& className, 
    QVector<pointer>& staticMemebers, 
    QVector<pointer>&& parents,
    int line) : 
    ANode(NodeType::Class, line), 
    functions(functions), 
    className(className), 
    staticMembers(staticMemebers),
    parents(parents)
{
}

QJsonObject Class::toJson() const
{
    QJsonObject ret;
    ret["type"] = "Class";
    ret["className"] = className;
    QJsonArray arr1;
    for (const auto& f : functions) {
        arr1.append(f->toJson());
    }
    ret["functions"] = arr1;
    QJsonArray arr2;
    for (const auto& m : staticMembers) {
        arr2.append(m->toJson());
    }
    ret["staticMembers"] = arr2;
    ret["line"] = getLine();
    QJsonArray arr3;
    for (const auto& p : parents) {
        arr3.append(p->toJson());
    }
    ret["parents"] = arr3;
    return ret;
}

Dict::Dict(QVector<QPair<pointer, pointer>>&& element, int line)
    : ANode(NodeType::Dict, line), elements(element)
{
}

QJsonObject Dict::toJson() const
{
    QJsonObject ret;
    ret["type"] = "Dict";
    QJsonArray arr;
    for (const auto& p : elements) {
        QJsonObject pair;
        pair["key"] = p.first->toJson();
        pair["value"] = p.second->toJson();
        arr.append(pair);
    }
    ret["elements"] = arr;
    ret["line"] = getLine();
    return ret;
}

Import::Import(pointer route, int line)
    : ANode(NodeType::Import, line), route(route)
{
}

QJsonObject Import::toJson() const
{
    QJsonObject ret;
    ret["type"] = "import";
    ret["route"] = route->toJson();
    ret["line"] = getLine();
    return ret;
}

Global::Global(pointer list, int line)
    : ANode(NodeType::Global, line), list(list)
{
}

QJsonObject Global::toJson() const
{
    QJsonObject ret;
    ret["type"] = "global";
    ret["list"] = list->toJson();
    ret["line"] = getLine();
    return ret;
}

}
