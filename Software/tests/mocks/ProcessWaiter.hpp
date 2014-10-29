#ifndef PROCESSWAITER_HPP
#define PROCESSWAITER_HPP

#pragma once

#include <QTimer>
#include <QMutex>
#include <QList>
#include <QProcess>
#include <QEventLoop>

class ProcessWaiter : public QObject {
    Q_OBJECT
public:
    ProcessWaiter(QObject * obj, const char * signal);

    Q_SLOT void stateChanged(QProcess::ProcessState state);
    void stop();
    bool wait(QProcess::ProcessState state, int timeout);
    bool isValid() const { return m_isValid; }

private:
    bool m_isValid;
    QTimer m_timer;
    QMutex m_mutex;
    QList<QProcess::ProcessState> m_stateList;
};

#endif // PROCESSWAITER_HPP
