#include "prefs.h"

#define PREFS_PATH	("resources/settings/prefs.ini")

Prefs::Prefs() : settings_(PREFS_PATH, QSettings::IniFormat) {
}

void Prefs::save(const QString& key, const QVariant& value) {
	settings_.setValue(key, value);
}

QVariant Prefs::load(const QString& key, const QVariant& defaultValue) {
	return settings_.value(key, defaultValue);
}
