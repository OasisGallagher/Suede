#pragma once
#include <QSettings>
#include "tools/singleton.h"

class Prefs : public singleton<Prefs> {
	friend class singleton<Prefs>;

public:
	void save(const QString& key, const QVariant& value);
	QVariant load(const QString& key, const QVariant& defaultValue = QVariant());

private:
	Prefs();

private:
	QSettings settings_;
};
