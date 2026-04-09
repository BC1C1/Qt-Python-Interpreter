#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qfiledialog.h"
#include "qfile.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initAll();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete core;
}

void MainWindow::initAll()
{
    // 全屏
    showMaximized();

    // 加载内核
    core = new Core();

    connectAllActions();

    setFont();
}

void MainWindow::connectAllActions()
{
    connectOpenAction();
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
    openFileAndToCodeEdit(path);
}

void MainWindow::openFileAndToCodeEdit(QString filePath)
{
    QFile f(filePath);
    f.open(QFile::ReadOnly | QFile::Text);
    QString content = f.readAll();
    ui->codeEditor->setPlainText(content);
}

void MainWindow::setDefaultFont()
{
    QFont font("Consolas", 14);
    ui->codeEditor->setFont(font);
}

void MainWindow::setFont()
{
    // if has config, config, or default, here only default
    setDefaultFont();
}

