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
        compileFunctions(ast);
        compileClasses(ast);
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
    for (const auto& s : block->statements) {
        auto type = s->getType();
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
        auto listParams = dynamicPointerCast<Parse::List>(functionStmt->listParams);
        auto block = functionStmt->block;
        // 1. name
        cache.push_back(Instruction{Code::LOAD_STRING, name}); // name
        QVector<Parse::pointer> posparams;
        QVector<Parse::pointer> keyparams;
        bool isTransed = false;
        for (const auto& e : listParams->elements) {
            switch (e->getType())
            {
            case NodeType::Variable: {
                if (isTransed)
                    throw std::runtime_error(u8"关键字参数不能先于位置参数");
                posparams.append(e);
                break;
            }
            case NodeType::Assignment: {
                if (((Parse::Assignment*)e.get())->left->getType() != NodeType::Variable)
                    throw std::runtime_error(u8"无效的函数参数");
                keyparams.append(e);
                isTransed = true;
                break;
            }
            default:
                throw std::runtime_error(u8"无效的函数参数");
                break;
            }
        }
        // 2. 首先是一个PList不变
        for (const auto& pose : posparams) {
            auto name = QString::fromStdString(((Parse::Variable*)(pose.get()))->name);
            cache.push_back(Instruction{ Code::LOAD_STRING, name });
        }
        cache.push_back(Instruction{ Code::LOAD_INT, posparams.size() });
        cache.push_back(Instruction{ Code::LOAD_LIST });
        // 3. 字典
        for (const auto& keye : keyparams) {
            auto assign = (Parse::Assignment*)keye.get();
            auto name = QString::fromStdString(((Parse::Variable*)(assign->left.get()))->name);
            cache.push_back(Instruction{ Code::LOAD_STRING, name });
            compileExpression(assign->right);
        }
        cache.push_back(Instruction{Code::LOAD_INT, keyparams.size()});
        cache.push_back(Instruction{Code::LOAD_DICT}); 
        // 4. 内部代码
        Compiler compiler;
        compiler.setAst(block);
        compiler.compileAST();
        auto& code = compiler.cache;
        code.pop_back(); // 弹出halt
        code.push_back(Instruction{ Code::RETURN });
        auto codeByte = Instruction::toByteArray(code);
        // 5. 是函数内部代码吗
        cache.push_back(Instruction{ Code::LOAD_FALSE });
        // 6. 创建
        cache.push_back(Instruction{Code::CREATE_FUNCTION, codeByte});
        // 7. 普通函数直接把函数对象绑定到name上
        cache.push_back(Instruction{Code::STORE_VAR, name});
        // stack(from b to top):
        // nameObj, ListObj(posParams), DictObj(keyParams), boolObj(isfunctioninclass)
    }
}

void Compiler::compileFunctionInClass(APointer node)
{
    auto functionStmt = dynamicPointerCast<Parse::Function>(node);
    auto name = QString::fromStdString(functionStmt->functionName);
    auto listParams = dynamicPointerCast<Parse::List>(functionStmt->listParams);
    auto block = functionStmt->block;

    cache.push_back(Instruction{ Code::LOAD_STRING, name }); // name
    QVector<Parse::pointer> posparams;
    QVector<Parse::pointer> keyparams;
    bool isTransed = false;
    for (const auto& e : listParams->elements) {
        switch (e->getType())
        {
        case NodeType::Variable: {
            if (isTransed)
                throw std::runtime_error(u8"关键字参数不能先于位置参数");
            posparams.append(e);
            break;
        }
        case NodeType::Assignment: {
            if (((Parse::Assignment*)e.get())->left->getType() != NodeType::Variable)
                throw std::runtime_error(u8"无效的函数参数");
            keyparams.append(e);
            isTransed = true;
            break;
        }
        default:
            throw std::runtime_error(u8"无效的函数参数");
            break;
        }
    }
    // 2. 首先是一个PList不变
    for (const auto& pose : posparams) {
        auto name = QString::fromStdString(((Parse::Variable*)(pose.get()))->name);
        cache.push_back(Instruction{ Code::LOAD_STRING, name});
    }
    cache.push_back(Instruction{ Code::LOAD_INT, posparams.size() });
    cache.push_back(Instruction{ Code::LOAD_LIST });
    // 3. 字典
    for (const auto& keye : keyparams) {
        auto assign = (Parse::Assignment*)keye.get();
        auto name = QString::fromStdString(((Parse::Variable*)(assign->left.get()))->name);
        cache.push_back(Instruction{ Code::LOAD_STRING, name });
        compileExpression(assign->right);
    }
    cache.push_back(Instruction{ Code::LOAD_INT, keyparams.size() });
    cache.push_back(Instruction{ Code::LOAD_DICT }); // params
    Compiler compiler;
    compiler.setAst(block);
    compiler.compileAST();
    auto& code = compiler.cache;
    code.pop_back();
    code.push_back(Instruction{ Code::RETURN });
    auto codeByte = Instruction::toByteArray(code);
    cache.push_back(Instruction{ Code::LOAD_TRUE });
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
        auto& parents = classDefine->parents;
        // 1. functions list
        for (const auto& f : functions) {
            compileFunctionInClass(f);
        }
        cache.push_back(Instruction{ Code::LOAD_INT, functions.size() });
        // 2. staticMembers list
        for (const auto& assign : staticMembers) {
            auto assignStmt = dynamicPointerCast<Parse::Assignment>(assign);
            compileExpression(assignStmt->right);
            if (assignStmt->left->getType() != NodeType::Variable)
                throw std::runtime_error(u8"意外的赋值");
            auto& varName = dynamicPointerCast<Parse::Variable>(assignStmt->left)->name;
            cache.push_back(Instruction{ Code::LOAD_STRING, QString::fromStdString(varName) });
        }
        cache.push_back(Instruction{ Code::LOAD_INT, staticMembers.size() });
        // 3. parents
        for (const auto& p : parents) {
            compileExpression(p);
        }
        cache.push_back(Instruction{ Code::LOAD_INT, parents.size() });
        // 4. create class
        cache.push_back(Instruction{ Code::CREATE_CLASS, name }); 
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
        QString attrName = QString::fromStdString(attrNode->attributeName);
        compileExpression(attrNode->caller);
        cache.push_back(Instruction{ Code::LOAD_ATTR, attrName });
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
    case NodeType::Dict: {
        auto dict = dynamicPointerCast<Parse::Dict>(node);
        for (const auto& pair : dict->elements) {
            compileExpression(pair.first);
            compileExpression(pair.second); // 这样第一个pop出来就是键
        }
        cache.push_back(Instruction{ Code::LOAD_INT, dict->elements.size() });
        cache.push_back(Instruction{ Code::LOAD_DICT });
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
        auto callStmt = (Parse::Call*)node.get();
        auto& left = callStmt->caller;
        if (left->getType() == NodeType::Variable) {
            auto caller = (Parse::Variable*)(left.get());
            if (caller->isSuper) {
                for (auto const& p : callStmt->params)
                    compileExpression(p);
                cache.push_back(Instruction{ Code::LOAD_INT, callStmt->params.size() });
                cache.push_back(Instruction{ Code::CREATE_SUPER });
                break;
            }
        }
        // 1.
        if (left->getType() == NodeType::Attribute)
            compileExpression(((Parse::Attribute*)(left.get()))->caller);
        else
            cache.push_back(Instruction{ Code::LOAD_NONE });
        QVector<Parse::pointer> posParams;
        QVector<Parse::pointer> keyParams;
        bool isTransed = false;
        for (const auto& e : callStmt->params) {
            if (e->getType() == NodeType::Assignment) {
                if (((Parse::Assignment*)e.get())->left->getType() != NodeType::Variable) {
                    throw std::runtime_error(u8"无效的参数");
                }
                isTransed = true;
                keyParams.append(e);
            }
            else {
                if (isTransed)
                    throw std::runtime_error(u8"关键字参数不能先于位置参数");
                posParams.append(e);
            }
        }

        // 2.
        for (const auto& pe : posParams) {
            compileExpression(pe);
        }
        cache.push_back(Instruction{ Code::LOAD_INT, posParams.size() });
        cache.push_back(Instruction{ Code::LOAD_LIST });
        // 3.
        for (const auto& ke : keyParams) {
            auto assign = (Parse::Assignment*)ke.get();
            compileExpression(assign->left);
            compileExpression(assign->right);
        }
        cache.push_back(Instruction{ Code::LOAD_INT, keyParams.size() });
        cache.push_back(Instruction{ Code::LOAD_DICT });
        // 4.
        compileExpression(callStmt->caller); 
        // 5.
        cache.push_back(Instruction{ Code::CALL });
        // caller's caller, listObj, dictObj, callerObj(func, class)
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
        /*        auto forStmt = dynamicPointerCast<Parse::For>(node);
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
*/
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
    //case NodeType::Call: {
    //    auto callStmt = dynamicPointerCast<Parse::Call>(node);
    //    auto& left = callStmt->caller;
    //    // 1.
    //    if (left->getType() == NodeType::Attribute)
    //        compileExpression(dynamicPointerCast<Parse::Attribute>(left)->caller);
    //    else
    //        cache.push_back(Instruction{ Code::LOAD_NONE });
    //    // 2.
    //    for (const auto& e : callStmt->listParams) {
    //        compileExpression(e);
    //    }
    //    cache.push_back(Instruction{ Code::LOAD_INT, callStmt->listParams.size() });
    //    cache.push_back(Instruction{ Code::LOAD_LIST });
    //    // 3.
    //    compileExpression(callStmt->caller); // 之前没讲全，也要把函数对象加载进栈
    //    // 4.
    //    cache.push_back(Instruction{ Code::CALL });
    //    break;
    //}
    case NodeType::Import: {
        auto importStmt = (Parse::Import*)(node.get());
        compileExpression(importStmt->route);
        cache.push_back(Instruction{ Code::IMPORT});
    }
    case NodeType::Class: {
        break;
    }
    default:
        compileExpression(node);
        break;
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


