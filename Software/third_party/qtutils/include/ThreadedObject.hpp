#ifndef THREADEDOBJECT_HPP
#define THREADEDOBJECT_HPP

#include <QThread>

#if !defined Q_NO_DEBUG
#include "RegisteredThread.hpp"
#endif

#include "QTUtils.hpp"

namespace QtUtils {

struct Location {
    Location(const char* loc, int line)
        : location(loc), line(line) {
    }

    const char* location;
    int line;
};

#define CURRENT_LOCATION Location(__FILE__, __LINE__)

template <TObj>
class ThreadedObject {
#if !defined Q_NO_DEBUG
    typedef QThread ThreadType;
#define THREAD_LOCATION
#else
    typedef RegisteredThread ThreadType;
#define THREAD_LOCATION(from) from.location, from.line
#endif

public:
    explicit ThreadedObject(TObj* object)
            : m_workingThread(THREAD_LOCATION(CURRENT_LOCATION)) {
        init(object);
    }

    ThreadedObject(const Location& from = CURRENT_LOCATION)
            : m_workingThread(THREAD_LOCATION(from)) {
        init(object);
    }

    void init(TObj* object) {
        Q_ASSERT(object);
        m_object = object;
        m_object->moveToThread(&m_workingThread);
        m_workingThread->start();
    }

    bool join(int ms) {
        m_object->deleteLater();
        m_workingThread.quit();
        m_object = NULL;
        return m_workingThread.wait(ms);
    }

    ~ThreadedObject() {
        Q_ASSERT(!m_object);
    }

private:
    TObj* m_object;
    ThreadType m_workingThread;
};

}

#endif // THREADEDOBJECT_HPP
