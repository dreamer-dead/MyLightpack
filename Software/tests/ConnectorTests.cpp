#include <QDebug>
#include <QScopedPointer>
#include <QCoreApplication>

#include "gtest/gtest.h"
#include "mocks/SignalAndSlotObject.hpp"
#include "third_party/qtutils/include/QTUtils.hpp"

namespace {
// Helper class to get access to protected fields of QtUtils::Connector.
template <typename T1, typename T2>
struct ConnectorFriend : protected QtUtils::Connector<T1, T2> {
public:
    typedef QtUtils::Connector<T1, T2> BaseType;
    typedef typename BaseType::SignallingType SignallingType;
    typedef typename BaseType::ReceivingType ReceivingType;

    static ReceivingType getSignalObj(const BaseType& connector) {
        return ConnectorFriend(connector).m_signallingObject;
    }

    static SignallingType getRecvObj(const BaseType& connector) {
        return ConnectorFriend(connector).m_receivingObject;
    }

private:
    ConnectorFriend(const BaseType& other) : BaseType(other) {}
};

// Actual helpers to get data form connectors.
// There is a way to get this with less amount of code:
// static_cast<const ConnectorFriend&>(connector).m_receivingObject
// But it's like a hack of C++ type system.
template <typename T1, typename T2>
inline typename QtUtils::Connector<T1, T2>::SignallingType
getSignalObj(const QtUtils::Connector<T1, T2>& connector) {
    return ConnectorFriend<T1, T2>::getSignalObj(connector);
}
template <typename T1, typename T2>
inline typename QtUtils::Connector<T1, T2>::ReceivingType
getRecvObj(const QtUtils::Connector<T1, T2>& connector) {
    return ConnectorFriend<T1, T2>::getRecvObj(connector);
}

struct PlainObject {};
}

TEST(ConnectorTests, CheckObjects) {
    const PlainObject obj = {};
    const QtUtils::Connector<PlainObject, PlainObject> connector(&obj);
    EXPECT_EQ(&obj, getSignalObj(connector));
    EXPECT_EQ(&obj, getRecvObj(connector));

    const PlainObject obj2 = {};
    const QtUtils::Connector<PlainObject, PlainObject> connector2(&obj, &obj2);
    EXPECT_EQ(&obj, getSignalObj(connector2));
    EXPECT_EQ(&obj2, getRecvObj(connector2));
}

TEST(ConnectorTests, CheckDefaultQObjects) {
    const QObject* const obj = reinterpret_cast<QObject*>(0xDEADBEEF);
    const QtUtils::Connector<> connector(obj);
    EXPECT_EQ(obj, getSignalObj(connector));
    EXPECT_EQ(obj, getRecvObj(connector));
}

TEST(ConnectorTests, CheckConnectionType) {
    const PlainObject obj = {};
    const PlainObject obj2 = {};
    const QtUtils::ConnectorWithType<PlainObject, PlainObject>
            connector(&obj, &obj2, Qt::QueuedConnection);
    EXPECT_EQ(&obj, getSignalObj(connector));
    EXPECT_EQ(&obj2, getRecvObj(connector));
    EXPECT_EQ(Qt::QueuedConnection, connector.type());
}

TEST(ConnectorTests, CheckMakers) {
    const PlainObject obj = {};
    const PlainObject obj2 = {};
    const QtUtils::Connector<PlainObject, PlainObject> connector(&obj, &obj2);
    const auto& copyConnector = QtUtils::makeConnector(&obj, &obj2);
    EXPECT_EQ(getSignalObj(copyConnector), getSignalObj(connector));
    EXPECT_EQ(getRecvObj(copyConnector), getRecvObj(connector));

    const QtUtils::ConnectorWithType<PlainObject, PlainObject>
            queuedConnector(&obj, &obj2, Qt::QueuedConnection);
    const auto& copyQueuedConnector = QtUtils::makeQueuedConnector(&obj, &obj2);
    EXPECT_EQ(getSignalObj(copyQueuedConnector), getSignalObj(queuedConnector));
    EXPECT_EQ(getRecvObj(copyQueuedConnector), getRecvObj(queuedConnector));
    EXPECT_EQ(copyQueuedConnector.type(), queuedConnector.type());
}

TEST(ConnectorTests, CheckSmartPointersMakers) {
    QScopedPointer<PlainObject> smartPointer(new PlainObject());
    QScopedPointer<PlainObject> smartPointer2(new PlainObject());
    const auto& connector = QtUtils::makeConnector(smartPointer, smartPointer2);
    EXPECT_EQ(smartPointer.data(), getSignalObj(connector));
    EXPECT_EQ(smartPointer2.data(), getRecvObj(connector));

    const auto& queuedConnector =
            QtUtils::makeQueuedConnector(smartPointer, smartPointer2);
    EXPECT_EQ(smartPointer.data(), getSignalObj(queuedConnector));
    EXPECT_EQ(smartPointer2.data(), getRecvObj(queuedConnector));
}

TEST(ConnectorTests, ConnectToSignalAsMember) {
    SignalAndSlotObject::State state;
    SignalAndSlotObject object(state);
    SignalAndSlotObject::State state2;
    SignalAndSlotObject object2(state2);
    const auto& connector = QtUtils::makeConnector(&object, &object2);
    connector.connect(&SignalAndSlotObject::fireSignal,
                      &SignalAndSlotObject::handleSignal);
    emit object.fireSignal();
    EXPECT_TRUE(state2.signalHandled);
}

TEST(ConnectorTests, ConnectToSINAL_SLOT_Macro) {
    SignalAndSlotObject::State state;
    SignalAndSlotObject object(state);
    SignalAndSlotObject::State state2;
    SignalAndSlotObject object2(state2);
    const auto& connector = QtUtils::makeConnector(&object, &object2);
    connector.connect(SIGNAL(fireSignal()), SLOT(handleSignal()));
    EXPECT_FALSE(state.signalHandled);
    emit object.fireSignal();
    EXPECT_TRUE(state2.signalHandled);
}

TEST(ConnectorTests, DeleteLater) {
    SignalAndSlotObject::State state;
    SignalAndSlotObject* object(new SignalAndSlotObject(state));
    QtUtils::deleteLaterOn(object, SIGNAL(fireSignal()));
    EXPECT_FALSE(state.wasDeleted);
    emit object->fireSignal();
    QCoreApplication::sendPostedEvents(object, QEvent::DeferredDelete);
    EXPECT_TRUE(state.wasDeleted);
}

