#include "main_window.h"
#include "ui_main_window.h"

#include "cmag_browser/target_graph_widget.h"

MainWindow::MainWindow(QWidget *parent, CmagProject &cmagProject)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    QWidget *tab = this->centralWidget()->findChild<QWidget *>("graph_container");
    QBoxLayout *layout = static_cast<QBoxLayout *>(tab->layout());

    TargetGraphWidget *graphWidget = new TargetGraphWidget(nullptr, cmagProject);
    layout->insertWidget(0, graphWidget);
}

MainWindow::~MainWindow() {
    delete ui;
}
