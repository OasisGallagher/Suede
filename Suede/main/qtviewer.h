#pragma once
#include <QObject>
#include <QtWidgets/QApplication>

#include "graphicsviewer.h"

class QtViewer : public QObject, public GraphicsViewer {
	Q_OBJECT

public:
	QtViewer(int argc, char *argv[]);

protected:
	virtual void Update();

private:
	void setup();

private slots:
	void canvasSizeChanged(uint w, uint h) { OnCanvasSizeChanged(w, h); }

private:
	QApplication app_;
};
