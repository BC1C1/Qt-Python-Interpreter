#include "compiler.h"
#include "core/utils/functions.h"

namespace Compile {
Compiler::Compiler(QObject *parent) : QObject(parent), cache()
{

}

void Compiler::setAst(APointer ast)
{
    this->ast = ast;
}

QVector<Instruction> Compiler::compileAST()
{
    cache.clear();
    try {
        compileBlock(ast);
    } catch (...) {
        throw;
    }
    cache.push_back(Instruction{Code::HALT});
    return cache;
}

void Compiler::compileBlock(APointer node)
{
    auto block = dynamicPointerCast<Parse::Block>(node);
    if (!node) {
        throw std::runtime_error(u8"无效的block");
    }
    compileFunctions(node);
    compileClasses(node);
    for (const auto& s : block->statements) {
        compileStatement(s);
    }
}

void Compiler::compileFunctions(APointer node)
{
    auto block = dynamicPointerCast<Parse::Block>(node);
    for (const auto& s : block->statements) {
        if (s->getType() != NodeType::FunctionDefine)
            continue;
        auto functionStmt = dynamicPointerCast<Parse::Function>(s);
        auto name = QString::fromStdString(functionStmt->functionName);
        auto params = dynamicPointerCast<Parse::List>(functionStmt->params);
        auto block = functionStmt->block;

        cache.push_back(Instruction{Code::LOAD_STRING, name}); // name
        for (const auto& e : params->elements) {
            auto eObj = dynamicPointerCast<Parse::Variable>(e);
            if (eObj)
                cache.push_back(Instruction{Code::LOAD_STRING, QString::fromStdString(eObj->name)});
            else
                throw std::runtime_error("无效的函数参数");
        }
        cache.push_back(Instruction{Code::LOAD_INT, params->elements.size()});
        cache.push_back(Instruction{Code::LOAD_LIST}); // params
        Compiler compiler;
        compiler.setAst(block);
        compiler.compileAST();
        auto code = compiler.cache;
        auto codeByte = Instruction::toByteArray(code);
        code.push_back(Instruction{Code::RETURN});
        cache.push_back(Instruction{Code::CREATE_FUNCTION, codeByte});
        cache.push_back(Instruction{Code::STORE_VAR, name});
    }
}

void Compiler::compileFunctionInClass(APointer node)
{
    auto functionStmt = dynamicPointerCast<Parse::Function>(node);
    auto name = QString::fromStdString(functionStmt->functionName);
    auto params = dynamicPointerCast<Parse::List>(functionStmt->params);
    auto block = functionStmt->block;

    cache.push_back(Instruction{ Code::LOAD_STRING, name }); // name
    for (const auto& e : params->elements) {
        auto eObj = dynamicPointerCast<Parse::Variable>(e);
        if (eObj)
            cache.push_back(Instruction{ Code::LOAD_STRING, QString::fromStdString(eObj->name) });
        else
            throw std::runtime_error("无效的函数参数");
    }
    cache.push_back(Instruction{ Code::LOAD_INT, params->elements.size() });
    cache.push_back(Instruction{ Code::LOAD_LIST }); // params
    Compiler compiler;
    compiler.setAst(block);
    compiler.compileAST();
    auto code = compiler.cache;
    auto codeByte = Instruction::toByteArray(code);
    code.push_back(Instruction{ Code::RETURN });
    cache.push_back(Instruction{ Code::CREATE_FUNCTION, codeByte });
    cache.push_back(Instruction{ Code::LOAD_STRING, name });
    //cache.push_back(Instruction{ Code::STORE_VAR, name });
}

void Compiler::compileClasses(APointer node)
{
    auto block = dynamicPointerCast<Parse::Block>(node);
    for (const auto& s : block->statements) {
        if (s->getType() != NodeType::Class)
            continue;
        auto classDefine = dynamicPointerCast<Parse::Class>(s);
        auto& name = classDefine->className;
        auto& functions = classDefine->functions;
        auto& staticMembers = classDefine->staticMembers;
        for (const auto& f : functions) {
            compileFunctionInClass(f);
        }
        cache.push_back(Instruction{ Code::LOAD_INT, functions.size() });
        cache.push_back(Instruction{ Code::LOAD_LIST });
        for (const auto& assign : staticMembers) {
            auto assignStmt = dynamicPointerCast<Parse::Assignment>(assign);
            compileExpression(assignStmt->right);
            if (assignStmt->left->getType() != NodeType::Variable)
                throw std::runtime_error(u8"意外的赋值");
            auto& varName = dynamicPointerCast<Parse::Variable>(assignStmt->left)->name;
            cache.push_back(Instruction{ Code::LOAD_STRING, QString::fromStdString(varName) });
        }
        cache.push_back(Instruction{ Code::LOAD_INT, staticMembers.size() });
        cache.push_back(Instruction{ Code::CREATE_CLASS, name }); // 这条指令会做你说的在当前环境赋值

    }
}

void Compiler::compileLeftValue(APointer node)
{
    auto type = node->getType();
    switch (type) {
    case NodeType::Variable: {
        break;
    }
    case NodeType::Index: {
        // 目标是加载索引对象和下标
        auto indexNode = dynamicPointerCast<Parse::Index>(node);
        auto toIndex = indexNode->obj;
        auto index = indexNode->expression;
        // 先搞对象
        compileExpression(toIndex);
        // 下标
        compileExpression(index);
        break;
    }
    case NodeType::Attribute: {
        auto attrNode = dynamicPointerCast<Parse::Attribute>(node);
        auto toAttr = attrNode->caller;
        compileExpression(toAttr);
        break;
    }
    default: {
        throw std::runtime_error(QString(u8"该结点不可作为左值, 结点类型: " + Parse::NodeTypeToQString(type)).toUtf8().data());
        break;
    }
    }
}

void Compiler::compileExpression(APointer node)
{
    auto type = node->getType();
    switch (type) {
    case NodeType::Variable: {
        auto var = dynamicPointerCast<Parse::Variable>(node);
        auto name = QString::fromStdString(var->name);
        cache.push_back(Instruction{Code::LOAD_NAME, QVariant(name)});
        break;
    }
    case NodeType::Index: {
        auto indexNode = dynamicPointerCast<Parse::Index>(node);
        compileExpression(indexNode->obj);
        compileExpression(indexNode->expression);
        break;
    }
    case NodeType::Attribute: {
        auto attrNode = dynamicPointerCast<Parse::Attribute>(node);
        compileExpression(attrNode->caller);
        break;
    }
    case NodeType::Int: {
        auto Int = dynamicPointerCast<Parse::Int>(node);
        auto value = Int->value;
        cache.push_back(Instruction{Code::LOAD_INT, QVariant(value)});
        break;
    }
    case NodeType::Float: {
        auto Float = dynamicPointerCast<Parse::Float>(node);
        auto value = Float->value;
        cache.push_back(Instruction{Code::LOAD_FLOAT, QVariant(value)});
        break;
    }
    case NodeType::String: {
        auto String = dynamicPointerCast<Parse::String>(node);
        auto value = QString::fromStdString(String->value);
        cache.push_back(Instruction{Code::LOAD_STRING, QVariant(value)});
        break;
    }
    case NodeType::List: {
        auto List = dynamicPointerCast<Parse::List>(node);
        for (const auto& ele : List->elements) {
            compileExpression(ele);
        }
        cache.push_back(Instruction{Code::LOAD_INT, List->elements.size()});
        cache.push_back(Instruction{Code::LOAD_LIST});
        break;
    }
    case NodeType::Binary: {
        auto binary = dynamicPointerCast<Parse::Binary>(node);
        compileExpression(binary->left);
        compileExpression(binary->right);
        switch (binary->op) {
        case Lex::TokenType::PLUS: {cache.push_back(Instruction{Code::ADD}); break;}
        case Lex::TokenType::MINUS: {cache.push_back(Instruction{Code::SUB}); break;}
        case Lex::TokenType::STAR: {cache.push_back(Instruction{Code::MUL}); break;}
        case Lex::TokenType::SLASH: {cache.push_back(Instruction{Code::DIV}); break;}
        case Lex::TokenType::MOD: {cache.push_back(Instruction{Code::MOD}); break;}
            //        case Lex::TokenType::POW: {cache.push_back(Instruction{Code::POW}); break;}
        case Lex::TokenType::EQ: {cache.push_back(Instruction{Code::EQ}); break;}
        case Lex::TokenType::NEQ: {cache.push_back(Instruction{Code::NEQ}); break;}
        case Lex::TokenType::GT: {cache.push_back(Instruction{Code::GT}); break;}
        case Lex::TokenType::GTE: {cache.push_back(Instruction{Code::GE}); break;}
        case Lex::TokenType::LTE: {cache.push_back(Instruction{Code::LE}); break;}
        case Lex::TokenType::LT: {cache.push_back(Instruction{Code::LT}); break;}

        default: {
            throw std::runtime_error(QString("%1不可做为操作数")
                                     .arg(Lex::Token::TypeToQStringStatic(binary->op))
                                     .toUtf8().data());
            break;
        }
        }
        break;
    }
    case NodeType::Call: {
        auto callStmt = dynamicPointerCast<Parse::Call>(node);
        for (const auto& e : callStmt->params) {
            compileExpression(e);
        }
        cache.push_back(Instruction{Code::LOAD_INT, callStmt->params.size()});
        cache.push_back(Instruction{Code::LOAD_LIST});
        compileExpression(callStmt->caller);
        cache.push_back(Instruction{Code::CALL});
        break;
    }
    default:{
        throwErrorLine(node->getLine());
        qDebug() << "意外的类型: " + Parse::NodeTypeToQString(type);
    }
    }
}

void Compiler::compileStatement(APointer node)
{
    auto type = node->getType();
    switch (type) {
    case NodeType::Assignment: {
        auto assignment = dynamicPointerCast<Parse::Assignment>(node);
        // compileLeftValue会生成这样一个指令，它的效果是运行后栈内会留有等待赋值的内容
        // 最简单的variable是不做处理的，这里就可以搞明白，输出storename即可，带一个string操作数
        // index会变成对象+下表，再压入值，然后输出storeindex即可
        // attribute会变成对象，在压入值（load），然后输出storeattr即可， 带一个attrname操作数
        compileExpression(assignment->right); // 压入值
        compileLeftValue(assignment->left);
        auto type = assignment->left->getType();
        switch (type) {
        case NodeType::Variable: {
            auto left = dynamicPointerCast<Parse::Variable>(assignment->left);
            auto name = QString::fromStdString(left->name);
            cache.push_back(Instruction{Code::STORE_VAR, name});
            break;
        }
        case NodeType::Index: {
            cache.push_back(Instruction{Code::STORE_INDEX});
            break;
        }
        case NodeType::Attribute: {
            auto attr = dynamicPointerCast<Parse::Attribute>(assignment->left);
            auto attrName = QString::fromStdString(attr->attributeName);
            cache.push_back(Instruction{Code::STORE_ATTR, attrName});
            break;
        }
        default: {
            throwErrorLine(assignment->getLine());
        }
        }
        break;
    }
    case NodeType::Print: {
        auto printStmt = dynamicPointerCast<Parse::Print>(node);
        try {
            compileExpression(printStmt->expression);
            cache.push_back(Instruction{Code::PRINT});
        } catch (std::runtime_error& e) {
            throwErrorLine(node->getLine());
            qDebug() << e.what();
        }
        break;
    }
    case NodeType::If: {
        QVector<int> needEndJumpIndex;
        int lastFalseJumpIndex;
        auto ifStmt = dynamicPointerCast<Parse::If>(node);
        auto ifCondition = ifStmt->condition;
        compileExpression(ifCondition);
        cache.push_back(Instruction{Code::JUMP_IF_FALSE});
        lastFalseJumpIndex = cache.size() - 1;
        compileBlock(ifStmt->block);
        cache.push_back(Instruction{Code::JUMP});
        needEndJumpIndex.push_back(cache.size() - 1);
        // 编译elifs
        for (const auto& elif : ifStmt->elifs) {
            auto elifStmt = dynamicPointerCast<Parse::If>(elif);
            auto elifCondition = elifStmt->condition;
            cache[lastFalseJumpIndex].operand = cache.size();
            compileExpression(elifCondition);
            cache.push_back(Instruction{Code::JUMP_IF_FALSE});
            lastFalseJumpIndex = cache.size() - 1;
            compileBlock(elifStmt->block);
            cache.push_back(Instruction{Code::JUMP});
            needEndJumpIndex.push_back(cache.size() - 1);
        }
        if (ifStmt->Else) {
            auto elseStmt = dynamicPointerCast<Parse::If>(ifStmt->Else);
            cache[lastFalseJumpIndex].operand = cache.size();
            compileBlock(elseStmt->block);
        } else {
            cache[lastFalseJumpIndex].operand = cache.size(); // 假如没有else语句，那么最后一个跳转位置就是末尾
        }

        int endPlace = cache.size();
        for (const auto& i : needEndJumpIndex) {
            cache[i].operand = endPlace;
        }
        break;
    }
    case NodeType::While: {
//        auto whileStmt = dynamicPointerCast<Parse::While>(node);
//        auto beginPlace = cache.size();
//        compileExpression(whileStmt->condition);
//        cache.push_back(Instruction{Code::JUMP_IF_FALSE, QVariant(cache.size())}); // 占位
//        int indexOfIfFalseJumpTo = cache.size() - 1;
//        auto ret = compileBlock(whileStmt->block);
//        cache.push_back(Instruction{Code::JUMP, QVariant(beginPlace)}); // sizevalue的最后一次修改, 所以不必要
//        auto loopEnd = cache.size();
//        cache[indexOfIfFalseJumpTo].operand = QVariant(cache.size());
//        for (const auto& p : ret) {
//            if (p.first == NeedType::Begin) {
//                cache[p.second].operand = beginPlace;
//            }
//            else if (p.first == NeedType::End){
//                cache[p.second].operand = loopEnd;
//            }
//        }
//        ret.clear();
//        break;
        auto whileStmt = dynamicPointerCast<Parse::While>(node);
        // 首先构造while块
        // 先压入两个int作为起始和终止位置
        cache.push_back(Instruction{Code::LOAD_INT});
        int beginIndex = cache.size() - 1;
        cache.push_back(Instruction{Code::LOAD_INT});
        int endIndex = cache.size() - 1;
        cache.push_back(Instruction{Code::LOOP_START_WHILE});
        auto condition = whileStmt->condition;
        int conditionBegin = cache.size();
        cache[beginIndex].operand = conditionBegin; // if continue or looping jump to here
        compileExpression(condition);
        cache.push_back(Code::JUMP_IF_FALSE);
        int ifFalseJumpToIndex = cache.size() - 1;
        compileBlock(whileStmt->block);
        cache.push_back(Instruction{Code::JUMP, conditionBegin});
        int endPlace = cache.size();
        cache[ifFalseJumpToIndex].operand = endPlace;
        cache[endIndex].operand = endPlace;
        cache.push_back(Instruction{Code::LOOP_WHILE_END});
        break;
    }
    case NodeType::For: {
//        auto forStmt = dynamicPointerCast<Parse::For>(node);
//        // iter = listObj.__iter__() 1
//        // label: here               2
//        // loopVar = iter.__next__() 3
//        // if loopVar is not None:   4
//        // do block                  5
//        // jump to here              6
//        auto varStdName = dynamicPointerCast<Parse::Variable>(forStmt->loopVar)->name;
//        QString varName = QString::fromStdString(varStdName);
//        compileExpression(forStmt->listObj);
//        cache.push_back(Instruction{Code::CREATE_ITER});
//        int herePlace = cache.size();
//        cache.push_back(Instruction{Code::ITER_NEXT, varName}); // 3
//        cache.push_back(Instruction{Code::LOAD_NAME, varName});
//        cache.push_back(Instruction{Code::LOAD_NONE});
//        cache.push_back(Instruction{Code::NEQ});
//        cache.push_back(Instruction{Code::JUMP_IF_FALSE});
//        int ifFalseJumpIndex = cache.size() - 1;
//        auto ret = compileBlock(forStmt->block);
//        cache.push_back(Instruction{Code::JUMP, herePlace});
//        int loopEnd = cache.size();
//        cache[ifFalseJumpIndex].operand = loopEnd;
//        cache.push_back(Instruction{Code::POP});
//        for (const auto& p : ret) {
//            if (p.first == NeedType::Begin) {
//                cache[p.second].operand = herePlace;
//            }
//            else if (p.first == NeedType::End){
//                cache[p.second].operand = loopEnd;
//            }
//        }
//        break;
        auto forStmt = dynamicPointerCast<Parse::For>(node);
        auto varStdName = dynamicPointerCast<Parse::Variable>(forStmt->loopVar)->name;
        QString varName = QString::fromStdString(varStdName);
        compileExpression(forStmt->listObj);
        cache.push_back(Code::CREATE_ITER);
        cache.push_back(Instruction{Code::LOAD_INT});
        int beginIndex = cache.size() - 1;
        cache.push_back(Instruction{Code::LOAD_INT});
        int endIndex = cache.size() - 1;
        cache.push_back(Instruction{Code::LOOP_START_FOR});
        int conditionBegin = cache.size();
        cache.push_back(Instruction{Code::ITER_NEXT, varName});
        cache.push_back(Instruction{Code::LOAD_NAME, varName});
        cache.push_back(Instruction{Code::LOAD_NONE});
        cache.push_back(Instruction{Code::NEQ});
        cache.push_back(Instruction{Code::JUMP_IF_FALSE});
        int indexOfIfFalseJumpTo = cache.size() - 1;
        compileBlock(forStmt->block);
        cache.push_back(Instruction{Code::JUMP, conditionBegin});
        int loopEnd = cache.size();
        cache[beginIndex].operand = conditionBegin;
        cache[endIndex].operand = loopEnd;
        cache[indexOfIfFalseJumpTo].operand = loopEnd;
        cache.push_back(Instruction{Code::LOOP_FOR_END});
        break;
    }
    case NodeType::Break: {
        cache.push_back(Instruction{Code::BREAK});
        break;
    }
    case NodeType::Continue: {
        cache.push_back(Instruction{Code::CONTINUE});
        break;
    }
    case NodeType::FunctionDefine: {
        break; // 执行遇到函数不管它，直接跳过
    }
    case NodeType::Return: {
        auto returnStmt = dynamicPointerCast<Parse::Return>(node);
        auto expression = returnStmt->expression;
        compileExpression(expression);
        cache.push_back(Instruction{Code::RETURN});
        break;
    }
    case NodeType::Class: {
        break;
    }
    default:
        throwErrorLine(node->getLine());
        throw std::runtime_error("无效的语句");
    }
}

void Compiler::throwErrorLine(int line1, int line2)
{
    QString msg;
    if (line2 == -1) {
        msg = QString(u8"Complie Error, Line %1 :").arg(line1);
    } else {
        msg = QString(u8"Complie Error, Between Line %1 and Line %2 :").arg(line1).arg(line2);
    }
    qDebug() << msg;
}


}


