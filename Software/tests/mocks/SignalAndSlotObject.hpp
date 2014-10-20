#ifndef SIGNALANDSLOTOBJECT_H
#define SIGNALANDSLOTOBJECT_H

#include <QObject>
#include <QString>

class SignalAndSlotObject : public QObject
{
    Q_OBJECT
public:
    struct State {
        bool signalHandled;
        QString stringSignalMessage;
        bool wasDeleted;
    };

    explicit SignalAndSlotObject(State& state, QObject *parent = 0);
    virtual ~SignalAndSlotObject();

    bool isSignalHandled() const { return m_state.signalHandled; }
    const QString& stringSignalMessage() const {
        return m_state.stringSignalMessage;
    }
    void reset();

signals:
    void fireSignal();
    void fireStringSignal(const QString& message);

public slots:
    void handleSignal();
    void handleStringSignal(const QString& message);

protected:
    State& m_state;
};

#endif // SIGNALANDSLOTOBJECT_H
