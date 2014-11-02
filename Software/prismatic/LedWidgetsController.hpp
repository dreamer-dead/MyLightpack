#ifndef LEDWIDGETSCONTROLLER_HPP
#define LEDWIDGETSCONTROLLER_HPP

#pragma once

#include <QtGui>
#include <QList>
#include <QRect>

class QWidget;
class GrabbedArea;
class GrabWidget;

class LedWidgetsController : public QObject {
    Q_OBJECT
public:
    LedWidgetsController(QWidget *parentWidget);
    ~LedWidgetsController();

    void initLedWidgets(int numberOfLeds);
    void toggleLedsVisibility(int numberOfLeds, bool isVisible);
    void showWidgets(bool show);
    void fillLedsBackgroundColored();
    void fillLedsBackgroundWhite();
    void scaleLedWidgets(const QRect& screenGeometry,
                         const QRect& prevScreenGeometry);

    bool inResizingOrMoving() const { return m_isPauseGrabWhileResizeOrMoving; }
    int widgetsCount() const { return m_ledWidgets.size(); }
    const GrabWidget& widget(int index) const { return *m_ledWidgets[index]; }
    QList<const GrabbedArea *>& grabbedAreas() { return m_ledWidgetsToAreas; }

private slots:
    void pauseWhileResizeOrMoving();
    void resumeAfterResizeOrMoving();

private:
    GrabWidget* createWidget() const;

    QWidget *m_parentWidget;
    bool m_isPauseGrabWhileResizeOrMoving;
    QList<GrabWidget *> m_ledWidgets;
    QList<const GrabbedArea *> m_ledWidgetsToAreas;
};

#endif // LEDWIDGETSCONTROLLER_HPP
