#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qfiledialog.h"
#include "qfile.h"
#include "qmessagebox.h"

#include "qtimer.h"
#include "Logger.h"
#include "newprojectwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), activeFilepath(QString()),
    core(nullptr), isNeedSave(false), newProjectWidget(nullptr)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete core;
}

// 用makeShared时自动调用，参见functions.h, 如果栈创建对象则显式调用这个函数
void MainWindow::initAll() 
{
    initLogger();

    // 全屏
    showMaximized();

    // 加载内核
    core = new Core();

    connectAll();

    setFont();
}

void MainWindow::initLogger()
{
    ui->outputPanel->setReadOnly(false);
    ui->outputPanel->setHtml("<div></div>");
    ui->outputPanel->setReadOnly(true);
    myStd::outputer_global = this;
    myStd::outputCallBack_global = [](const myStd::info& info) -> void {
        QString text = std2qt(info.text);
        QColor color;

        switch (info.level) {
        case myStd::LogLevel::DEBUG_:
#ifndef DEBUG_MODE
            return;
#endif
            color = Qt::gray;
            break;

        case myStd::LogLevel::INFO_:
#ifndef INFO_MOD
            return;
#endif
            color = Qt::black;
            break;

        case myStd::LogLevel::WARNING_:
#ifndef PRINT_MOD
            return;
#endif
            color = Qt::blue;
            break;

        case myStd::LogLevel::ERROR_:
#ifndef ERROR_MOD
            return;
#endif
            color = Qt::red;
            break;

        default:
            color = Qt::black;
            break;
        }

        QMetaObject::invokeMethod(qApp, [=]() {
            auto win = (MainWindow*)myStd::outputer_global;

            QTextCursor cursor(win->ui->outputPanel->document());
            cursor.movePosition(QTextCursor::End);

            QTextCharFormat format;
            format.setForeground(color);
            cursor.setCharFormat(format);

            cursor.insertText(text);
            cursor.insertText("\n");
            }, Qt::QueuedConnection);
        };
}

void MainWindow::connectAll()
{
    connectAllActions();
    /*connect(newProjectWidget, &NewProjectWidget::createSuccessful, this, &MainWindow::onCreateSuccessful);*/
}

void MainWindow::connectAllActions()
{
    connectOpenAction();
    connectRunAction();
    connectSaveAction();
    connectNewProjectAction();
    connectNewFile();
}

void MainWindow::connectNewProjectAction()
{
    connect(ui->actionnew_project, &QAction::triggered, this, &MainWindow::newProjectActionTriggered);
}

void MainWindow::newProjectActionTriggered()
{
    newProjectWidget = newWindow<NewProjectWidget>(this);

    newProjectWidget->setWindowFlags(Qt::Window);

    newProjectWidget->setWindowModality(Qt::WindowModal);

    newProjectWidget->show();
}

void MainWindow::connectSaveAction()
{
    connect(ui->actionsave, &QAction::triggered, this, &MainWindow::saveActionTriggered);
    connect(ui->codeEditor, &QTextEdit::textChanged, this, &MainWindow::setNeedSave);
}

void MainWindow::saveActionTriggered()
{
    if (!this->isNeedSave || this->activeFilepath.isEmpty()) return;
    auto ret = QMessageBox::question(
        this,                       
        "提示",                     
        "确定保存？",        
        QMessageBox::Yes | QMessageBox::No  
    );
    if (ret != QMessageBox::Yes) return;
    QString tempPath = activeFilepath + ".tmp";
    QFile tempFile(tempPath);
    if (!tempFile.open(QFile::WriteOnly | QFile::Text)) {
        log("保存失败：无法创建临时文件");
        return;
    }

    QByteArray content = ui->codeEditor->toPlainText().toUtf8();
    if (tempFile.write(content) != content.size()) {
        log("保存失败：写入临时文件出错");
        tempFile.close();
        QFile::remove(tempPath); 
        return;
    }
    tempFile.close(); 

    QFile::remove(activeFilepath);
    if (!QFile::rename(tempPath, activeFilepath)) {
        log("保存失败：临时文件替换原文件出错");
        QFile::remove(tempPath);
        return;
    }
    log("文件保存成功：" + activeFilepath);
    this->isNeedSave = false;
}

void MainWindow::connectOpenAction()
{
    connect(ui->actionopen, &QAction::triggered, this, &MainWindow::openActionTriggered);
}

void MainWindow::openActionTriggered()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        "选择要打开的文件",
        "",
        "",
        nullptr,
        QFileDialog::DontUseNativeDialog
    );
    if (!path.isEmpty())
        openFileAndToCodeEdit(path);
    this->isNeedSave = false;
}

void MainWindow::connectRunAction()
{
    connect(ui->actionrun, &QAction::triggered, this, &MainWindow::runActionTriggered);
}

void MainWindow::runActionTriggered()
{
    if (isNeedSave)
        saveActionTriggered();
    if (activeFilepath.isEmpty())
        return;
    try {
        core->execute(activeFilepath);
    }
    catch (LexerError& e) {
        // 不应该有
    }
    catch (ParserError& e) {
        // 未来应当提示错误
    }
    catch (CompilerError& e) {
        // 不应该有
    }
    catch (VMError& e) {
        // 未来应当提示错误
    }
}

void MainWindow::connectNewFile()
{
    connect(ui->actionnew_file, &QAction::triggered, this, &MainWindow::newFileTriggered);
}

void MainWindow::newFileTriggered()
{
    if (activeFilepath.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("提示");
        msgBox.setText("是否创建新项目");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        if (ret)
            newProjectActionTriggered();
        return;
    }
    // 否则创建新文件

}

void MainWindow::onCreateSuccessful(QString root, QString srcPath, QString activeFile)
{
    openFileAndToCodeEdit(activeFile);
}

bool MainWindow::showConfirmDialog(const QString& title, const QString& text)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);       // 标题
    msgBox.setText(text);               // 提示文本
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    int ret = msgBox.exec();

    return (ret == QMessageBox::Yes);
}

void MainWindow::openFileAndToCodeEdit(QString filePath)
{
    QFile f(filePath);
    activeFilepath = filePath;
    if (!f.open(QFile::ReadOnly | QFile::Text)) { 
        log("无法打开文件：" + filePath);
        return;
    }
    QString content = f.readAll();
    ui->codeEditor->setPlainText(content);
    f.close();
}

void MainWindow::setDefaultFont()
{
    QFont font("Microsoft YaHei", 14);
    ui->codeEditor->setFont(font);
    ui->outputPanel->setFont(font);
}

void MainWindow::setFont()
{
    // if has config, config, or default, here only default
    setDefaultFont();
}

void MainWindow::setNeedSave()
{
    this->isNeedSave = true;
}

void MainWindow::closeEvent(QCloseEvent*)
{
    this->saveActionTriggered();
}

