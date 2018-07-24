#pragma once

#include <QDockWidget>
#include "../winbase.h"

class Lighting : public QDockWidget, public WinSingleton<Lighting> {
	Q_OBJECT

public:
	Lighting(QWidget* parent);

public:
	virtual void init(Ui::Suede* ui);

protected:
	virtual void showEvent(QShowEvent* event);

private slots:
	void onAmbientChanged(const QColor& color);
};
