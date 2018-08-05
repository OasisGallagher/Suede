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

public:
	static QString skinName();
	static bool setSkin(const QString& name);
	static QList<QString> builtinSkinNames();

protected:
	virtual void Update();
	virtual void PollEvents();

private:
	void setupSuede();
	void setupRegistry();

private slots:
	void onAboutToCloseSuede() { Close(); }
	void canvasSizeChanged(uint w, uint h) { OnCanvasSizeChanged(w, h); }

private:
	Suede* suede_;
	QApplication app_;

private:
	static QMap<QString, QString> skinResources;
};
