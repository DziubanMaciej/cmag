#include "target_graph_widget.h"

#include "cmag_lib/core/cmag_project.h"

#include <QMouseEvent>
#include <QPainter>

TargetGraphWidget::TargetGraphWidget(QWidget *parent, CmagProject &cmagProject)
    : QWidget(parent),
      cmagProject(cmagProject) {
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setMouseTracking(true);
}

QSize TargetGraphWidget::sizeHint() const {
    return {100, 100};
}

QSize TargetGraphWidget::minimumSizeHint() const {
    return {100, 100};
}

void TargetGraphWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setTransform(camera.transform);
    QPen normalPen{};
    QPen focusedPen{};
    focusedPen.setColor(Qt::green);
    QPen selectedPen{};
    selectedPen.setColor(Qt::blue);
    for (const CmagTarget &target : cmagProject.getTargets()) {
        drawTarget(target, painter, normalPen, focusedPen, selectedPen);
    }
}

void TargetGraphWidget::mousePressEvent(QMouseEvent *event) {
    if (targetDrag.active || cameraDrag.active) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (focusedTarget) {
            targetDrag.active = true;
            targetDrag.target = focusedTarget;
            targetDrag.startPoint = event->position();
        }

        if (selectedTarget != focusedTarget) {
            selectedTarget = focusedTarget;
            emit setSelectedTarget(selectedTarget);
        }

        repaint();
    }

    if (event->button() == Qt::MiddleButton) {
        cameraDrag.active = true;
        cameraDrag.lastPoint = event->position();
    }
}

void TargetGraphWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (targetDrag.active && event->button() == Qt::LeftButton) {
        targetDrag.target->graphical.x += static_cast<float>(targetDrag.offset.x());
        targetDrag.target->graphical.y += static_cast<float>(targetDrag.offset.y());
        targetDrag = {};
        repaint();
    }
    if (cameraDrag.active && event->button() == Qt::MiddleButton) {
        cameraDrag = {};
    }
}

void TargetGraphWidget::mouseMoveEvent(QMouseEvent *event) {
    const QPointF mousePosition = event->position();
    updateFocusedTarget(mousePosition);

    if (targetDrag.active) {
        QPointF offset = mousePosition - targetDrag.startPoint;
        offset /= camera.scale;

        targetDrag.offset += offset;
        targetDrag.startPoint = mousePosition;
        repaint();
    }

    if (cameraDrag.active) {
        QPointF offset = mousePosition - cameraDrag.lastPoint;
        offset /= camera.scale;

        camera.move += offset;
        cameraDrag.lastPoint = mousePosition;
        updateCamera();
    }
}

void TargetGraphWidget::wheelEvent(QWheelEvent *event) {
    const float delta = static_cast<float>(event->angleDelta().y());
    if (delta > 0) {
        camera.scale *= 1.25;
    } else if (delta < 0) {
        camera.scale /= 1.25;
    } else {
        return;
    }

    camera.scale = std::clamp(camera.scale, qreal(0.6), qreal(1.5));

    printf("scale=%f\n", (float)camera.scale);
    updateCamera();
}

void TargetGraphWidget::updateFocusedTarget(QPointF mousePosition) {
    CmagTarget *result = nullptr;
    for (CmagTarget &target : cmagProject.getTargets()) {
        if (isPointInTarget(target, mousePosition)) {
            result = &target;
            break;
        }
    }
    if (result != focusedTarget) {
        focusedTarget = result;
        repaint();
    }
}

void TargetGraphWidget::updateCamera() {
    camera.transform.reset();
    camera.transform.scale(camera.scale, camera.scale);
    camera.transform.translate(camera.move.x(), camera.move.y());
    repaint();
}

bool TargetGraphWidget::isPointInTarget(const CmagTarget &target, QPointF point) const {
    point = camera.transform.inverted().map(point); // TODO inverting matrix...

    float minX = target.graphical.x - 50;
    float maxX = target.graphical.x + 50;
    float minY = target.graphical.y - 50;
    float maxY = target.graphical.y + 50;
    if (targetDrag.active && targetDrag.target == &target) {
        minX += static_cast<float>(targetDrag.offset.x());
        maxX += static_cast<float>(targetDrag.offset.x());
        minY += static_cast<float>(targetDrag.offset.y());
        maxY += static_cast<float>(targetDrag.offset.y());
    }

    return minX <= point.x() && point.x() <= maxX &&
           minY <= point.y() && point.y() <= maxY;
}

void TargetGraphWidget::drawTarget(
    const CmagTarget &target,
    QPainter &painter,
    QPen &normalPen,
    QPen &focusedPen,
    QPen &selectedPen) const {

    QPointF offset = {};
    if (targetDrag.active && targetDrag.target == &target) {
        offset = targetDrag.offset;
    }

    QRectF rect = {};
    rect.setLeft(target.graphical.x + offset.x() - 50);
    rect.setRight(target.graphical.x + offset.x() + 50);

    rect.setTop(target.graphical.y + offset.y() - 50);
    rect.setBottom(target.graphical.y + offset.y() + 50);

    if (selectedTarget == &target) {
        painter.setPen(selectedPen);
    } else if (focusedTarget == &target) {
        painter.setPen(focusedPen);
    } else {
        painter.setPen(normalPen);
    }
    painter.drawRect(rect);
    painter.drawText(rect, Qt::AlignmentFlag::AlignCenter, QString(target.name.c_str()), &rect);
}
