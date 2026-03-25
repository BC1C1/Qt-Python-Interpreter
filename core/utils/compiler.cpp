#include "compiler.h"

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
        auto ret = compileBlock(ast);
    } catch (...) {
        throw;
    }
    cache.push_back(Instruction{Code::HALT});
    return cache;
}

pairVector Compiler::compileBlock(APointer node)
{
    auto block = dynamicPointerCast<Parse::Block>(node);
    if (!node) {
        throw std::runtime_error(u8"无效的block");
    }
    pairVector ret;
    for (const auto& s : block->statements) {
        auto temp = compileStatement(s);
        for (auto const& t : temp)
            ret.push_back(t);
    }
    return ret;
}

//void Compiler::compileLeftValue(APointer node)
//{
//    auto type = node->getType();
//    switch (type) {
//    case NodeType::Variable: {
//        break;
//    }
//    case NodeType::Index: {
//        // 目标是加载索引对象和下标
//        auto indexNode = dynamicPointerCast<Parse::Index>(node);
//        auto toIndex = indexNode->obj;
//        auto index = indexNode->expression;
//        // 先搞对象
//        compileExpression(toIndex);
//        // 下标
//        compileExpression(index);
//        break;
//    }
//    case NodeType::Attribute: {
//        auto attrNode = dynamicPointerCast<Parse::Attribute>(node);
//        auto toAttr = attrNode->caller;
//        compileExpression(toAttr);
//        break;
//    }
//    default: {
//        throw std::runtime_error(QString(u8"该结点不可作为左值, 结点类型: " + Parse::NodeTypeToQString(type)).toUtf8().data());
//        break;
//    }
//    }
//}

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
    default:{
        throwErrorLine(node->getLine());
        qDebug() << "意外的类型: " + Parse::NodeTypeToQString(type);
    }
    }
}

pairVector Compiler::compileStatement(APointer node)
{
    pairVector globalRet;
    auto type = node->getType();
    switch (type) {
    case NodeType::Assignment: {
        auto assignment = dynamicPointerCast<Parse::Assignment>(node);
        // compileLeftValue会生成这样一个指令，它的效果是运行后栈内会留有等待赋值的内容
        // 最简单的variable是不做处理的，这里就可以搞明白，输出storename即可，带一个string操作数
        // index会变成对象+下表，再压入值，然后输出storeindex即可
        // attribute会变成对象，在压入值（load），然后输出storeattr即可， 带一个attrname操作数
        compileExpression(assignment->left);
        compileExpression(assignment->right); // 压入值
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
        auto ifBlockret = compileBlock(ifStmt->block);
        globalRet.append(ifBlockret);
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
            auto elifBlockRet = compileBlock(elifStmt->block);
            globalRet.append(elifBlockRet);
            cache.push_back(Instruction{Code::JUMP});
            needEndJumpIndex.push_back(cache.size() - 1);
        }
        if (ifStmt->Else) {
            auto elseStmt = dynamicPointerCast<Parse::If>(ifStmt->Else);
            cache[lastFalseJumpIndex].operand = cache.size();
            auto elseBlockRet = compileBlock(elseStmt->block);
            globalRet.append(elseBlockRet);
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
        auto whileStmt = dynamicPointerCast<Parse::While>(node);
        auto beginPlace = cache.size();
        compileExpression(whileStmt->condition);
        cache.push_back(Instruction{Code::JUMP_IF_FALSE, QVariant(cache.size())}); // 占位
        int indexOfIfFalseJumpTo = cache.size() - 1;
        auto ret = compileBlock(whileStmt->block);
        cache.push_back(Instruction{Code::JUMP, QVariant(beginPlace)}); // sizevalue的最后一次修改, 所以不必要
        auto loopEnd = cache.size();
        cache[indexOfIfFalseJumpTo].operand = QVariant(cache.size());
        for (const auto& p : ret) {
            if (p.first == NeedType::Begin) {
                cache[p.second].operand = beginPlace;
            }
            else {
                cache[p.second].operand = loopEnd;
            }
        }
        ret.clear();
        break;
    }
    case NodeType::For: {
        auto forStmt = dynamicPointerCast<Parse::For>(node);
        // iter = listObj.__iter__()
        // label: here
        // loopVar = iter.__next__()
        // if loopVar is not None:
        // do block
        // jump to here
        compileExpression(forStmt->listObj);
        cache.push_back(Instruction{Code::CREATE_ITER});
        int herePlace = cache.size();



        break;
    }
    case NodeType::Break: {
        auto needInput = cache.size();
        cache.push_back(Instruction{Code::JUMP, QVariant(0)}); // 占位
        globalRet.push_back(qMakePair<NeedType, int>(NeedType::End, needInput));
        break;
    }
    case NodeType::Continue: {
        auto needInput = cache.size();
        cache.push_back(Instruction{Code::JUMP, QVariant(0)});
        globalRet.push_back(qMakePair<NeedType, int>(NeedType::Begin, needInput));
        break;
    }
    default:
        throwErrorLine(node->getLine());
        throw std::runtime_error("无效的语句");
    }
    return globalRet;
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


