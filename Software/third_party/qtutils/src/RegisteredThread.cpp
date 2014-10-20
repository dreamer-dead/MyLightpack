#include "RegisteredThread.hpp"

#include <QDebug>

namespace {
static size_t g_threadsCount = 0;
}  // namespace

namespace QtUtils {
size_t registerThread(const QThread* thread, const char* location, int line) {
    ++g_threadsCount;
    qDebug() << "registerThread(" << thread << " at " << location << ":" << line
             << "), count = " << g_threadsCount;
    return g_threadsCount;
}

void deregisterThread(const QThread* thread) {
    --g_threadsCount;
    qDebug() << "deregisterThread(" << thread << "), count =" << g_threadsCount;
}

size_t registeredThreadsCount() {
    return g_threadsCount;
}
}
