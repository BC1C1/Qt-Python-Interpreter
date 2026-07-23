#ifndef NEWPROJECTWIDGET_H
#define NEWPROJECTWIDGET_H

#include <QWidget>
#include "SecondaryWindow.h"

namespace Ui {
class NewProjectWidget;
}

class NewProjectWidget : public QWidget, public SecondaryWindow
{
    Q_OBJECT

public:
    explicit NewProjectWidget(QWidget *parent = nullptr);
    ~NewProjectWidget();
public:
    void clearAll();
public:
    void initAll();
    
    void connectAll();

    void connectReturnButton();
    void onReturnButtonClicked();

    void connectSelectPathButton();
    void onSelectButtonClicked();

    void connectCreateButton();
    void onCreateButtonClicked();

public:
    virtual void connectToMainWindow() override;
private:
    Ui::NewProjectWidget *ui;
signals:
    void createSuccessful(QString root, QString srcPath, QString activeFile);
};

#endif // NEWPROJECTWIDGET_H
