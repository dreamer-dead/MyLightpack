#ifndef GRABTESTS_HPP
#define GRABTESTS_HPP

#include <QObject>

class GrabTests : public QObject
{
    Q_OBJECT
public:
    explicit GrabTests(QObject *parent = 0);

private slots:
    void testCase_GrabContextTest();
};

#endif // GRABTESTS_HPP
