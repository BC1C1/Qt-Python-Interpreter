#include "mainwindow.h"

#include <QApplication>
#include <qdebug.h>
#include "core/utils/lexer.h"

Q_DECLARE_METATYPE(Lex::Token)
Q_DECLARE_METATYPE(QVector<Lex::Token>)

int main(int argc, char *argv[])
{
    std::string text = R"(
a = 1 + 2
print(a)


)";
    Lex::Lexer lexer;
    auto result = lexer.scanTokens(text);
    for (auto& t: result) {
        qDebug() << t.TokenToQString(t);
    }
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
