#ifndef COMPILER_H
#define COMPILER_H

#include <QObject>

class Compiler : public QObject
{
    Q_OBJECT
public:
    explicit Compiler(QObject *parent = nullptr);

signals:

};

#endif // COMPILER_H
