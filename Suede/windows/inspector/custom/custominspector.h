#pragma once
#include <QGroupBox>
#include <QFormLayout>

#include "object.h"

#define DEFINE_LITERAL(name)	static const char* name = #name

class QListWidget;
class CustomInspector : public QGroupBox {
public:
	CustomInspector(const QString& title, Object object);

public:
	static QString float2QString(float f);

protected:
	void resizeGeometryToFit(QListWidget* w);

	QString formatRowName(const QString& name) const;

protected:
	Object target_;
	QFormLayout* form_;
};
