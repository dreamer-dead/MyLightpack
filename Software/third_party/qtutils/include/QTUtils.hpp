#ifndef QTUTILS_HPP
#define QTUTILS_HPP

#include <QObject>
#include <QString>
#include <QScopedPointer>

namespace QtUtils {
QString pathCombine(const QString& path1, const QString& path2);

template <typename T1 = QObject, typename T2 = QObject>
class Connector {
public:
    typedef const T1* SignallingType;
    typedef const T2* ReceivingType;

    explicit Connector(const T1* signalObj)
        : m_signallingObject(signalObj), m_receivingObject(signalObj) {
        Q_ASSERT(signalObj);
    }

    Connector(const T1* signalObj, const T2* recvObj)
        : m_signallingObject(signalObj), m_receivingObject(recvObj) {
        Q_ASSERT(m_signallingObject);
        Q_ASSERT(m_receivingObject);
    }

    template <typename Func1, typename Func2>
    const Connector& connect(Func1 signal, Func2 slot) const {
        QObject::connect(m_signallingObject, signal, m_receivingObject, slot);
        return *this;
    }

    template <typename Func1, typename Func2>
    const Connector& disconnect(Func1 signal, Func2 slot) const {
        QObject::disconnect(m_signallingObject, signal, m_receivingObject, slot);
        return *this;
    }

protected:
    const T1* const m_signallingObject;
    const T2* const m_receivingObject;
};

template <typename T1 = QObject, typename T2 = QObject>
class ConnectorWithType : public Connector<T1, T2> {
public:
    ConnectorWithType(const T1* signalObj, Qt::ConnectionType type)
        : Connector<T1, T2>(signalObj), m_type(type) {
    }

    ConnectorWithType(const T1* signalObj, const T2* recvObj, Qt::ConnectionType type)
        : Connector<T1, T2>(signalObj, recvObj), m_type(type) {
    }

    template <typename Func1, typename Func2>
    const ConnectorWithType& connect(Func1 signal, Func2 slot) const {
        QObject::connect(this->m_signallingObject, signal, this->m_receivingObject, slot, m_type);
        return *this;
    }

    template <typename Func1, typename Func2>
    const ConnectorWithType& disconnect(Func1 signal, Func2 slot) const {
        QObject::disconnect(this->m_signallingObject, signal, this->m_receivingObject, slot, m_type);
        return *this;
    }

    Qt::ConnectionType type() const { return m_type; }

private:
    const Qt::ConnectionType m_type;
};

template <typename T1, typename T2>
inline Connector<T1, T2> makeConnector(const T1* signalObj, const T2* recvObj) {
    return Connector<T1, T2>(signalObj, recvObj);
}

template <typename T1, typename T2>
inline Connector<T1, T2> makeConnector(const QScopedPointer<T1>& signalObj,
                                       const QScopedPointer<T2>& recvObj) {
    return Connector<T1, T2>(signalObj.data(), recvObj.data());
}

template <typename T1, typename T2>
inline ConnectorWithType<T1, T2> makeQueuedConnector(
        const T1* signalObj, const T2* recvObj) {
    return ConnectorWithType<T1, T2>(signalObj, recvObj, Qt::QueuedConnection);
}

template <typename T1, typename T2>
inline ConnectorWithType<T1, T2> makeQueuedConnector(
        const QScopedPointer<T1>& signalObj,
        const QScopedPointer<T2>& recvObj) {
    return ConnectorWithType<T1, T2>(signalObj.data(), recvObj.data(), Qt::QueuedConnection);
}

template <typename T, typename Func>
inline void deleteLaterOn(const T* signalObj, Func signal) {
    Connector<T, T>(signalObj).connect(signal, SLOT(deleteLater()));
}

template <typename T, typename Func>
inline void deleteLaterOn(const QScopedPointer<T>& signalObj, Func signal) {
    Connector<T, T>(signalObj.data()).connect(signal, SLOT(deleteLater()));
}
}

#endif // QTUTILS_HPP
