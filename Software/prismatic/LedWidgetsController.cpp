#include "LedWidgetsController.hpp"

#include "PrismatikMath.hpp"
#include "common/DebugOut.hpp"
#include "ui/GrabWidget.hpp"

LedWidgetsController::LedWidgetsController(QWidget *parentWidget)
    : QObject(parentWidget)
    , m_parentWidget(parentWidget)
    , m_isPauseGrabWhileResizeOrMoving(false) {
}

LedWidgetsController::~LedWidgetsController() {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    for (int i = 0; i < m_ledWidgets.size(); ++i) {
        delete m_ledWidgets[i];
    }

    m_ledWidgets.clear();
    m_ledWidgetsToAreas.clear();
}

void LedWidgetsController::pauseWhileResizeOrMoving() {
    DEBUG_MID_LEVEL << Q_FUNC_INFO;
    m_isPauseGrabWhileResizeOrMoving = true;
}

void LedWidgetsController::resumeAfterResizeOrMoving() {
    DEBUG_MID_LEVEL << Q_FUNC_INFO;
    m_isPauseGrabWhileResizeOrMoving = false;
}

GrabWidget* LedWidgetsController::createWidget() const {
    GrabWidget * ledWidget = new GrabWidget(m_ledWidgets.size(), m_parentWidget);

    QObject::connect(ledWidget, &GrabWidget::resizeOrMoveStarted,
                     this, &LedWidgetsController::pauseWhileResizeOrMoving);
    QObject::connect(ledWidget, &GrabWidget::resizeOrMoveCompleted,
                     this, &LedWidgetsController::resumeAfterResizeOrMoving);
    return ledWidget;
}

void LedWidgetsController::initLedWidgets(int numberOfLeds) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << numberOfLeds;

    if (m_ledWidgets.isEmpty()) {
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "First widget initialization";

        GrabWidget * const ledWidget = createWidget();

// TODO: Check out this line!
//         First LED widget using to determine grabbing-monitor in WinAPI version of Grab
//        connect(ledWidget, SIGNAL(resizeOrMoveCompleted(int)), this, SLOT(firstWidgetPositionChanged()));

        m_ledWidgets << ledWidget;
    }

    int diff = numberOfLeds - m_ledWidgets.size();
    if (diff > 0) {
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Append" << diff << "grab widgets";

        for (int i = 0; i < diff; ++i) {
            GrabWidget * const ledWidget = createWidget();
            m_ledWidgets << ledWidget;
        }
    } else {
        diff *= -1;
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Remove last" << diff << "grab widgets";

        while (diff --> 0)
        {
            delete m_ledWidgets.last();
            m_ledWidgets.removeLast();
        }
    }

    m_ledWidgetsToAreas.clear();
    for (int i = 0; i < m_ledWidgets.size(); ++i) {
        m_ledWidgetsToAreas << m_ledWidgets[i];
    }

    if (m_ledWidgets.size() != numberOfLeds) {
        qCritical() << Q_FUNC_INFO
            << "Fail: m_ledWidgets.size()" << m_ledWidgets.size()
            << " != numberOfLeds" << numberOfLeds;
    }
}

void LedWidgetsController::toggleLedsVisibility(int numberOfLeds, bool isVisible)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << numberOfLeds << isVisible;

    initLedWidgets(numberOfLeds);

    for (int i = 0; i < m_ledWidgets.size(); ++i) {
        m_ledWidgets[i]->settingsProfileChanged();
        m_ledWidgets[i]->setVisible(isVisible);
    }
}

void LedWidgetsController::showWidgets(bool show) {
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << show;

    typedef void (GrabWidget::* ShowMethod)();
    const ShowMethod method = show ? &GrabWidget::show : &GrabWidget::hide;
    for (int i = 0; i < m_ledWidgets.size(); ++i) {
        (m_ledWidgets[i]->*method)();
    }
}

void LedWidgetsController::fillLedsBackgroundColored() {
    for (int i = 0; i < m_ledWidgets.size(); i++)
        m_ledWidgets[i]->fillBackgroundColored();
}

void LedWidgetsController::fillLedsBackgroundWhite() {
    for (int i = 0; i < m_ledWidgets.size(); ++i)
        m_ledWidgets[i]->fillBackgroundWhite();
}

void LedWidgetsController::scaleLedWidgets(const QRect& screenGeometry,
                                           const QRect& prevScreenGeometry) {
    using PrismatikMath::round;

    // Move LedWidgets
    const int deltaX = prevScreenGeometry.x() - screenGeometry.x();
    const int deltaY = prevScreenGeometry.y() - screenGeometry.y();
    const double scaleX = (double)screenGeometry.width() / prevScreenGeometry.width();
    const double scaleY = (double)screenGeometry.height() / prevScreenGeometry.height();

    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "deltaX =" << deltaX << "deltaY =" << deltaY;
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "scaleX =" << scaleX << "scaleY =" << scaleY;

    for(int i=0; i < m_ledWidgets.size(); ++i) {
        Q_ASSERT(m_ledWidgets[i]);
        GrabWidget& widget = *m_ledWidgets[i];
        if (!prevScreenGeometry.contains(widget.geometry().center()))
            continue;

        const int width  = round(scaleX * widget.width());
        const int height = round(scaleY * widget.height());

        int x = widget.x();
        int y = widget.y();

        x -= screenGeometry.x();
        y -= screenGeometry.y();

        x = round(scaleX * x);
        y = round(scaleY * y);

        x += screenGeometry.x();
        y += screenGeometry.y();

        x -= deltaX;
        y -= deltaY;

        widget.move(x,y);
        widget.resize(width, height);

        widget.saveSizeAndPosition();

        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "new values [" << i << "]" << "x =" << x << "y =" << y << "w =" << width << "h =" << height;
    }
}

