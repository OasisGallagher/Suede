#include "prefs.h"
#include <QSettings>

#define PREFS_PATH	("resources/settings/prefs.ini")

static QSettings settings(PREFS_PATH, QSettings::IniFormat);

void Prefs::save(const QString& key, const QVariant& value) {
	settings.setValue(key, value);
}

QVariant Prefs::load(const QString& key, const QVariant& defaultValue) {
	return settings.value(key, defaultValue);
}
