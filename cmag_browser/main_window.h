#pragma once

#include <QMainWindow>

class CmagProject;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent, CmagProject &cmagProject);
    ~MainWindow() override;

private:
    void insertWidget(const char *containerName, QWidget *widget);

    Ui::MainWindow *ui;
};
