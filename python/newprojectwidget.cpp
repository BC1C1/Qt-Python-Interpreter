#include "newprojectwidget.h"
#include "ui_newprojectwidget.h"
#include "Project.h"
#include "Exception.h"
#include "QFileDialog"
#include "MainWindow.h"

NewProjectWidget::NewProjectWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewProjectWidget)
{
    ui->setupUi(this);
    initAll();
}

NewProjectWidget::~NewProjectWidget()
{
    delete ui;
}

void NewProjectWidget::clearAll()
{
    ui->projectPathLine->clear();
    ui->projectNameLine->clear();
}

void NewProjectWidget::initAll()
{
    connectAll();
}

void NewProjectWidget::connectAll()
{
    connectReturnButton();
    connectCreateButton();
    connectSelectPathButton();
}

void NewProjectWidget::connectReturnButton()
{
    connect(ui->NoPBtn, &QPushButton::clicked, this, &NewProjectWidget::onReturnButtonClicked);
}

void NewProjectWidget::onReturnButtonClicked()
{
    this->close();
}

void NewProjectWidget::connectSelectPathButton()
{
    connect(ui->selectPathPBtn, &QPushButton::clicked, this, &NewProjectWidget::onSelectButtonClicked);
}

void NewProjectWidget::onSelectButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(
        this,
        "选择项目保存路径",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly
    );

    if (!path.isEmpty()) {
        ui->projectPathLine->setText(path);
    }
}

void NewProjectWidget::connectCreateButton()
{
    connect(ui->yesPBtn, &QPushButton::clicked, this, &NewProjectWidget::onCreateButtonClicked);
}

void NewProjectWidget::onCreateButtonClicked()
{
    auto projectName = ui->projectNameLine->text();
    auto route = ui->projectPathLine->text();
    QString ProPath = QDir(route).filePath(projectName);
    QString srcPath = QDir(ProPath).filePath("src");
    auto filePath = QDir(srcPath).filePath("main.py");
    try {
        auto pro = Project::createProject(projectName, route, false);

        emit createSuccessful(pro.proPath, pro.srcPath, filePath);
    }
    catch (const InvalidPath& e) {
        // 路径无效异常：项目名称为空、基础路径为空、父路径为空、目录名称为空、文件夹名称为空等
        QString errMsg = QString("路径无效, %1").arg(e.what());
        logError(errMsg);
    }
    catch (const FileAlreadyExists& e) {
        // 文件已存在异常：项目已存在
        QString errMsg = QString("项目已存在, %1").arg(e.what());
        logError(errMsg);
    }
    catch (const FileNotFound& e) {
        // 文件不存在异常：父路径不存在
        QString errMsg = QString("父路径不存在, %1").arg(e.what());
        logError(errMsg);
    }
    catch (const NotADirectory& e) {
        // 不是目录异常：路径不是文件夹
        QString errMsg = QString("路径不是文件夹, %1").arg(e.what());
        logError(errMsg);
    }
    catch (const DirectoryCreationFailed& e) {
        // 目录创建失败异常：创建目录失败、创建文件夹失败
        QString errMsg = QString("路径不是文件夹, %1").arg(e.what());
        logError(errMsg);
    }
    catch (const HiddenAttributeFailed& e) {
        // 隐藏属性设置失败异常：Windows 平台设置隐藏属性失败
        QString errMsg = QString("Windows 平台设置隐藏属性失败, %1").arg(e.what());
        logError(errMsg);
    }
    catch (const RenameFailed& e) {
        // 重命名失败异常：Unix/Linux/macOS 平台重命名为隐藏文件夹失败
        QString errMsg = QString("Unix/Linux/macOS 平台重命名为隐藏文件夹失败, %1").arg(e.what());
        logError(errMsg);
    }
    catch (const FileError& e) {
        // 文件错误基类：捕获其他未明确列出的文件相关异常
        QString errMsg = QString("捕获其他未明确列出的文件相关异常, %1").arg(e.what());
        logError(errMsg);
    }
    catch (const std::exception& e) {
        // 标准异常：捕获其他标准异常
        QString errMsg = QString("捕获其他标准异常, %1").arg(e.what());
        logError(errMsg);
    }
    catch (...) {
        logError("未知的错误");
    }
    this->close();
}

void NewProjectWidget::connectToMainWindow()
{
    auto mwin = qobject_cast<MainWindow*>(this->parent());
    connect(this, &NewProjectWidget::createSuccessful, mwin, &MainWindow::onCreateSuccessful);
}
