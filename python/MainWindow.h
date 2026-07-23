#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Core.h"

class NewProjectWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public:
    void initAll();
private:
    void initLogger();
    void connectAll();
private:
    void connectAllActions();

    void connectNewProjectAction();
    void newProjectActionTriggered();

    void connectSaveAction();
    void saveActionTriggered();

    void connectOpenAction();
    void openActionTriggered();

    void connectRunAction();
    void runActionTriggered();

    void connectNewFile();
    void newFileTriggered();
public:
    void onCreateSuccessful(QString root, QString srcPath, QString activeFile);
private:
    void openFileAndToCodeEdit(QString filePath);
private:
    void setDefaultFont();
    void setFont();

    bool showConfirmDialog(const QString& title, const QString& text);
public:
    void setNeedSave();
protected:
    virtual void closeEvent(QCloseEvent*) override;
private:
    Ui::MainWindow *ui;
    Core* core;
    QString activeFilepath;
    bool isNeedSave;
    NewProjectWidget* newProjectWidget;
};
#endif // MAINWINDOW_H
