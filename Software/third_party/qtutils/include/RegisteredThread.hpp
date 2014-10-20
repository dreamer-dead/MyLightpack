#ifndef REGISTEREDTHREAD_HPP
#define REGISTEREDTHREAD_HPP

#include <QThread>

namespace QtUtils {
size_t registerThread(const QThread*, const char* name, int line);
void deregisterThread(const QThread*);
size_t registeredThreadsCount();

class RegisteredThread : public QThread {
    Q_OBJECT
public:
    RegisteredThread(const char* name, int line) {
        registerThread(this, name, line);
    }

    virtual ~RegisteredThread() {
        deregisterThread(this);
    }
};
}

#endif // REGISTEREDTHREAD_HPP
