#pragma once

#include <QWidget>

class CmagProject;
class CmagTarget;

class TargetGraphWidget : public QWidget {
    Q_OBJECT

public:
    TargetGraphWidget(QWidget *parent, CmagProject &project);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    CmagProject &cmagProject;
    CmagTarget *focusedTarget = {};
    struct TargetDrag {
        bool active = {};
        QPointF startPoint = {};
        CmagTarget *target = {};
        QPointF offset = {};
    } targetDrag = {};
    struct CameraDrag {
        bool active = {};
        QPointF lastPoint = {};
    } cameraDrag;
    struct Camera {
        QPointF move = {};
        qreal scale = 1.0;
        QTransform transform = {};
    } camera;

    void updateFocusedTarget(QPointF mousePosition);
    void updateCamera();
    bool isPointInTarget(const CmagTarget &target, QPointF point) const;
    void drawTarget(const CmagTarget &target, QPainter &painter, QPen &unfocusedPen, QPen &focusedPen) const;
};
