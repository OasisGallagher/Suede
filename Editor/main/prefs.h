#pragma once
#include <QVariant>

class Prefs {
public:
	static void save(const QString& key, const QVariant& value);
	static QVariant load(const QString& key, const QVariant& defaultValue = QVariant());
};
