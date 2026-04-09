#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Core.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void initAll();
private:
    void connectAllActions();

    void connectOpenAction();
    void openActionTriggered();

    void connectRunAction();
    void runActionTriggered();
private:
    void openFileAndToCodeEdit(QString filePath);
private:
    void setDefaultFont();
    void setFont();
private:
    Ui::MainWindow *ui;
    Core* core;
    QString activeFilepath;
};
#endif // MAINWINDOW_H
