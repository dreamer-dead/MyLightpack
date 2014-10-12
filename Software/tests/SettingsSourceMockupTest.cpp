#include "SettingsSourceMockup.hpp"
#include "gtest/gtest.h"

TEST(SettingsSourceMockupTest, AddValue) {
	SettingsSourceMockup settingsSource;
	const QString key("SomeKey");
	const QVariant value(100500);

	EXPECT_FALSE(settingsSource.contains(key));
	settingsSource.setValue(key, value);
	EXPECT_TRUE(settingsSource.contains(key));
	EXPECT_EQ(settingsSource.value(key), value);
}

TEST(SettingsSourceMockupTest, RemoveValue) {
	SettingsSourceMockup settingsSource;
	const QString key("SomeKey");
	const QVariant value(100500);

	settingsSource.setValue(key, value);
	EXPECT_TRUE(settingsSource.contains(key));
	settingsSource.remove(key);
	EXPECT_FALSE(settingsSource.contains(key));
}
