#include "main_window.h"
#include "ui_main_window.h"

#include "cmag_browser/target_graph_widget.h"
#include "cmag_browser/target_properties_widget.h"

MainWindow::MainWindow(QWidget *parent, CmagProject &cmagProject)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    auto graphWidget = new TargetGraphWidget(nullptr, cmagProject);
    auto propertiesWidget = new TargetPropertiesWidget(nullptr);
    QObject::connect(graphWidget, &TargetGraphWidget::setSelectedTarget, propertiesWidget, &TargetPropertiesWidget::setSelectedTarget);

    insertWidget("graph_container", graphWidget);
    insertWidget("properties_container", propertiesWidget);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::insertWidget(const char *containerName, QWidget *widget) {
    QWidget *tab = this->centralWidget()->findChild<QWidget *>(containerName);
    QBoxLayout *layout = static_cast<QBoxLayout *>(tab->layout());
    layout->insertWidget(0, widget);
}
