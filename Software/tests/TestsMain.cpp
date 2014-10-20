#include <iostream>
#include <QApplication>
#include <QTimer>

#include "common/DebugOut.hpp"
#include "gtest/gtest.h"

using namespace std;

unsigned g_debugLevel = Debug::LowLevel;

namespace {
class TestApplication : public QApplication {
    Q_OBJECT
public:
    TestApplication(int &argc, char **argv) : QApplication(argc, argv) {
        testing::InitGoogleTest(&argc, argv);
    }

    int exec() const {
        QTimer::singleShot(0, this, SLOT(onEventLoopStarted()));
        return QApplication::exec();
    }

public Q_SLOTS:
    void onEventLoopStarted() {
        cout << "Run Lightpack tests:" << endl;
        QCoreApplication::exit(RUN_ALL_TESTS());
    }
};
}

#include "TestsMain.moc"

int main(int argc, char *argv[])
{
    TestApplication app(argc, argv);
    return app.exec();
}
