#pragma once
#include <QGroupBox>
#include <QFormLayout>
#include <QColorDialog>

#include "object.h"

#define DEFINE_LITERAL(name)	static const char* name = #name

class QListWidget;
class CustomInspector : public QGroupBox {
	Q_OBJECT

public:
	CustomInspector(const QString& title, Object object);

protected:
	void resizeGeometryToFit(QListWidget* w);
	QString formatRowName(const QString& name) const;

protected:
	Object target_;
	QFormLayout* form_;
};
