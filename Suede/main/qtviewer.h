#pragma once
#include <QObject>
#include <QtWidgets/QApplication>

#include "graphicsviewer.h"

class Suede;
class QtViewer : public QObject, public GraphicsViewer {
	Q_OBJECT

public:
	QtViewer(int argc, char *argv[]);
	~QtViewer();

protected:
	virtual void Update();

private:
	void setupStyle();
	void setupSuede();
	void setupRegistry();

private slots:
	void onAboutToCloseSuede() { Close(); }
	void canvasSizeChanged(uint w, uint h) { OnCanvasSizeChanged(w, h); }

private:
	Suede* suede_;
	QApplication app_;
};