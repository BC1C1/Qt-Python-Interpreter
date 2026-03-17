#ifndef PVM_H
#define PVM_H

#include <QObject>
#include <QVector>
#include <QDebug>
#include <QStack>
#include "core/objects/runtime/pobject.h"
#include "core/utils/functions.h"
#include "core/objects/runtime/pint.h"
#include "core/objects/runtime/pstr.h"
#include "core/objects/runtime/environment.h"
// code define begin
namespace vm {
enum class Code {
    // 加载字面量
    LOAD_INT,
    LOAD_FLOAT,
    LOAD_STRING,

    // 加载变量
    LOAD_NAME,
    STORE_NAME,

    // 算数
    ADD,


    // 打印测试
    PRINT,

    // 停机
    HALT,

};

QString CodeToQString(Code code);
//Code QStringToCode(QString string);

// 测试用
//QVector<Code> fromTextToVector(const QString& text);


// code define end

struct Instruction {
    Code code;
    QVariant operand;

    Instruction(Code c, const QVariant& op = QVariant())
        : code(c), operand(op) {}
};

using namespace Py;
using pointer = Py::PObject::pointer;
using ValueStack = QStack<pointer>;
using EPointer = QSharedPointer<Environment>;
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
private:
    void throwErrMsg(const std::string& msg);
private:
    QVector<Instruction> codes;
    int PC;
    bool isRunning;
    ValueStack valueStack;
    EPointer globalEnvir;
signals:

};
}


#endif // PVM_H
