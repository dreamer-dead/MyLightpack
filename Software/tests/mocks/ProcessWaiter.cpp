#include "ProcessWaiter.hpp"

#include <QDebug>

ProcessWaiter::ProcessWaiter(QObject * obj, const char * signal) {
    m_isValid = connect(obj, signal, SLOT(stateChanged(QProcess::ProcessState)), Qt::QueuedConnection);
}

void ProcessWaiter::stateChanged(QProcess::ProcessState state) {
    {
        QMutexLocker locker(&m_mutex);
        m_stateList << state;
    }
    stop();
}

void ProcessWaiter::stop() {
  m_timer.stop();
  QMetaObject::invokeMethod(&m_timer, "timeout");
}

bool ProcessWaiter::wait(QProcess::ProcessState state, int timeout) {
    {
        QMutexLocker locker(&m_mutex);
        if (!m_stateList.isEmpty()) {
            bool getState = m_stateList.front() == state;
            m_stateList.pop_front();
            return getState;
        }
    }

    QEventLoop loop;
    m_timer.start(timeout);
    loop.connect(&m_timer, SIGNAL(timeout()), SLOT(quit()));
    loop.exec();
    bool getState = false;
    {
        QMutexLocker locker(&m_mutex);
        if (!m_stateList.isEmpty()) {
            getState = m_stateList.front() == state;
            m_stateList.pop_front();
        }
    }
    return getState;
}

