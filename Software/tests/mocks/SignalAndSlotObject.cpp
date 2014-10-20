#include <QDebug>

#include "SignalAndSlotObject.hpp"

SignalAndSlotObject::SignalAndSlotObject(
    SignalAndSlotObject::State& state, QObject *parent)
    : QObject(parent)
    , m_state(state) {
    reset();
}

SignalAndSlotObject::~SignalAndSlotObject() {
    qDebug() << Q_FUNC_INFO;
    m_state.wasDeleted = true;
}

void SignalAndSlotObject::handleSignal() {
    qDebug() << Q_FUNC_INFO;
    m_state.signalHandled = true;
}

void SignalAndSlotObject::handleStringSignal(const QString& message) {
    m_state.signalHandled = true;
    m_state.stringSignalMessage = message;
}

void SignalAndSlotObject::reset() {
    m_state.signalHandled = false;
    m_state.stringSignalMessage.clear();
    m_state.wasDeleted = false;
}
