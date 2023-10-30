#pragma once

#include <QFormLayout>
#include <QWidget>

class CmagTarget;

class TargetPropertiesWidget : public QWidget {
    Q_OBJECT

public:
    explicit TargetPropertiesWidget(QWidget *parent);

public slots:
    void setSelectedTarget(CmagTarget *target);

private:
    QFormLayout *propertiesFormLayout;
    CmagTarget *target = nullptr;

    static void clearLayout(QLayout *layout); // TODO move to some shared helper
};
