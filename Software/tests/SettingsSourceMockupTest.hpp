#ifndef SETTINGSSOURCEMOCKUPTEST_H
#define SETTINGSSOURCEMOCKUPTEST_H

#include <QTest>

class SettingsSourceMockupTest : public QObject
{
	Q_OBJECT
public:
	explicit SettingsSourceMockupTest(QObject *parent = 0);

public slots:
	void testCase_AddValue();
	void testCase_RemoveValue();
};

#endif // SETTINGSSOURCEMOCKUPTEST_H
