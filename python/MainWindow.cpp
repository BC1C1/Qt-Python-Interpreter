#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qfiledialog.h"
#include "qfile.h"

#include "qtimer.h"
#include "Logger.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), activeFilepath(QString())
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
    myStd::outputCallBack_global = [](const char* text) {
        QString safeText = std2qt(text);

        QMetaObject::invokeMethod(qApp, [=]() {
            for (QWidget* widget : qApp->topLevelWidgets()) {
                if (auto* win = qobject_cast<MainWindow*>(widget)) {
                    win->ui->outputPanel->append(safeText);
                    break;
                }
            }
            }, Qt::QueuedConnection);
        };

    myStd::log("✅ Logger 已成功切换到 Qt 界面输出！");

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
    connectRunAction();
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

void MainWindow::connectRunAction()
{
    connect(ui->actionrun, &QAction::triggered, this, &MainWindow::runActionTriggered);
}

void MainWindow::runActionTriggered()
{
    core->execute(activeFilepath);
}

void MainWindow::openFileAndToCodeEdit(QString filePath)
{
    QFile f(filePath);
    activeFilepath = filePath;
    f.open(QFile::ReadOnly | QFile::Text);
    QString content = f.readAll();
    ui->codeEditor->setPlainText(content);
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

