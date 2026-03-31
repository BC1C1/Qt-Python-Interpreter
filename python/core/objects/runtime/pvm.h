#ifndef PVM_H
#define PVM_H

#include <QObject>
#include <QVector>
#include <QDebug>
#include <QStack>
#include <QFile>
#include <QHash>
#include <QByteArray>
#include <QDataStream>
#include "core/objects/runtime/pobject.h"
#include "core/objects/runtime/blockandframe.h"
// code define begin
namespace vm {
enum class Code: char {
    // 加载字面量
    LOAD_INT,           // need 数字           栈：[] → [值]
    LOAD_TRUE,
    LOAD_FALSE,
    LOAD_FLOAT,         // need 数字           栈：[] → [值]
    LOAD_STRING,        // need 字符串         栈：[] → [值]
    LOAD_LIST,          //                    栈：[] → [值]
    LOAD_NONE,          //                    栈：[] → [值]

    // 加载变量
    LOAD_NAME,          // need 变量名         栈：[] → [值]
    LOAD_INDEX,         // need 无             栈：[对象, 下标] → [值]
    LOAD_ATTR,          // need 属性名         栈：[对象] → [属性值]

    // 赋值
    STORE_VAR,          // need 变量名         栈：[值] → []
    STORE_INDEX,        // need 无             栈：[对象, 下标, 值] → []
    STORE_ATTR,         // need 属性名         栈：[对象, 值] → []

    // 跳转
    JUMP_IF_FALSE,      // 弹栈，false则跳转, need where to jump
    JUMP,               // 无条件跳转, need, need where to jump

    // 迭代器
    CREATE_ITER,        // 从栈中取obj，尝试获取其迭代器压入栈中
    ITER_NEXT,          // 从栈中获取迭代器，调用next, 并赋值给参数，参数来自于指令的操作数, 不取出iter

    // 循环
    LOOP_START_FOR,     // 从栈中取出end和begin地址，然后以此构造LOOPFORFRAME
    LOOP_START_WHILE,   // 从栈中取出end和begin地址，然后以此构造LOOPWHILEFRAME
    CONTINUE,           // top块栈，直到当前块为for|while块，检查后跳转
    BREAK,              // top块栈，直到当前块为for|while块，随后检查后清理迭代器并跳转，然后pop块栈
    LOOP_FOR_END,       // 只看当前块
    LOOP_WHILE_END,


    // 函数
    CREATE_FUNCTION,
    RETURN,
    CALL,

    // 类定义
    CREATE_CLASS,
    STORE_CLASS_VAR,

    // 算数
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    POW,

    // 比较
    EQ,
    NEQ,
    GT,
    GE,
    LT,
    LE,

    POP,

    // 打印测试
    PRINT,

    // 停机
    HALT,

    INVALID

};

QString CodeToQString(Code code);
Code QStringToCode(QString string);

// 测试用
//QVector<Code> fromTextToVector(const QString& text);


// code define end
struct Instruction {
    Code code;
    QVariant operand;

    Instruction() {}
    Instruction(Code c, const QVariant& op = QVariant())
        : code(c), operand(op) {}
    QString toString() const;

    static QVector<Instruction> fromByteArray(const QByteArray& array);
    static QByteArray toByteArray(const QVector<Instruction>& ins);
};

bool saveInstructionsToTXT(const QVector<Instruction>& instructions, const QString& filename);
QVector<Instruction> loadInstructionsFromTXT(const QString& filename);

using namespace Py;
using pointer = Py::PObject::pointer;
using ValueStack = QStack<pointer>;
using EPointer = QSharedPointer<Environment>;
using BlockFrameStack = QStack<BlockFrame>;
using CallFrameStack = QStack<CallFrame>;
class PVM : public QObject
{
    Q_OBJECT
public:
    PVM(const QVector<Instruction>& codes, QObject *parent = nullptr);
    void start();
    void stop();
    void run();
private:
    void executeSingleCode();
    void pushValue(pointer object);
    pointer popValue();
    pointer topValue();
    pointer getBasicObject(QVariant value);
    void dumpStack(const QString& hint = "");
    EPointer createNewEnvironment(EPointer currEnvir);
    BlockFrame popFrame();
    void pushFrame(const BlockFrame& blockFrame);
    BlockFrame topFrame();
    EPointer currEnvir();
private:
    void throwErrMsg(const std::string& msg);
private:
    QVector<Instruction> currCodes;
    int PC;
    bool isRunning;
    ValueStack valueStack;
    ValueStack funcStack;
//    EPointer currEnvir;
//    EPointer defaultEnvir;
    BlockFrameStack blockFrameStack;
    CallFrameStack callFrameStack;
signals:

};
}
inline QDataStream& operator<<(QDataStream& out, const vm::Instruction& ins)
{
    out << (qint8)ins.code;
    out << ins.operand;
    return out;
}

inline QDataStream& operator>>(QDataStream& in, vm::Instruction& ins)
{
    qint8 c;
    in >> c;
    ins.code = (vm::Code)c;
    in >> ins.operand;
    return in;
}

#endif // PVM_H
