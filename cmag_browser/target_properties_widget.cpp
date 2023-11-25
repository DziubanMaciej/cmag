#include "target_properties_widget.h"

#include "cmag_lib/core/cmag_project.h"

#include <QLabel>
#include <QLineEdit>
#include <QSpacerItem>
#include <QVBoxLayout>

TargetPropertiesWidget::TargetPropertiesWidget(QWidget *parent)
    : QWidget(parent),
      propertiesFormLayout(new QFormLayout()) {

    this->setLayout(new QVBoxLayout(this));
    this->layout()->addWidget(new QLineEdit("A"));

    QWidget *w = new QWidget();
    w->setLayout(propertiesFormLayout);
    this->layout()->addWidget(w);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
}

void TargetPropertiesWidget::setSelectedTarget(CmagTarget *newTarget) {
    target = newTarget;

    clearLayout(propertiesFormLayout);

    if (target) {
        propertiesFormLayout->addRow(new QLabel("name"), new QLabel(target->name.c_str()));

        const CmagTargetConfig &config = newTarget->configs[0]; // TODO select proper config
        for (const CmagTargetProperty &property : config.properties) {
            QLabel *nameLabel = new QLabel(property.name.c_str());
            nameLabel->setStyleSheet("QLabel { background-color : red; color : blue; }");
            QLabel *valueLabel = new QLabel(property.value.c_str());
            propertiesFormLayout->addRow(nameLabel, valueLabel);
        }
    }
}

void TargetPropertiesWidget::clearLayout(QLayout *layout) {

    while (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        if (QWidget *widget = item->widget())
            widget->deleteLater();
        if (QLayout *childLayout = item->layout())
            clearLayout(childLayout);
        delete item;
    }
}