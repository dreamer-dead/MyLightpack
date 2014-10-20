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

#define CURRENT_LOCATION QtUtils::Location(__FILE__, __LINE__)

template <typename TObj>
class ThreadedObject {
#if defined Q_NO_DEBUG
    typedef QThread ThreadType;
#define THREAD_LOCATION(from)
#else
    typedef RegisteredThread ThreadType;
#define THREAD_LOCATION(from) from.location, from.line
#endif

public:
    explicit ThreadedObject(TObj* object)
            : m_object(NULL)
            , m_workingThread(THREAD_LOCATION(CURRENT_LOCATION)) {
        init(object);
    }

    ThreadedObject(const Location& from = CURRENT_LOCATION)
            : m_object(NULL)
            , m_workingThread(THREAD_LOCATION(from)) {
    }

    inline TObj* get() const { return m_object; }

    TObj* operator ->() const {
        Q_ASSERT(m_object);
        return m_object;
    }

    const QThread& thread() const { return m_workingThread; }

    void init(TObj* object) {
        Q_ASSERT(object);
        m_object = object;
        m_object->moveToThread(&m_workingThread);
        m_workingThread.start();
    }

    bool join(int ms) {
        if (m_object)
            m_object->deleteLater();
        m_workingThread.quit();
        m_object = NULL;
        return m_workingThread.wait(ms);
    }

    ~ThreadedObject() {
        Q_ASSERT(!m_object);
        Q_ASSERT(m_workingThread.isFinished());
    }

private:
    TObj* m_object;
    ThreadType m_workingThread;
};

}

#endif // THREADEDOBJECT_HPP
