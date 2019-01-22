#pragma once
#include <QObject>
#include <QtWidgets/QApplication>

#include "graphicsviewer.h"

class Editor;

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

private:
	void setupEditor();
	void setupRegistry();

private slots:
	void onAboutToCloseEditor() { Close(); }
	void canvasSizeChanged(uint w, uint h) { OnCanvasSizeChanged(w, h); }

private:
	Editor* editor_;
	QApplication app_;

private:
	static QString defaultSkin;
	static QMap<QString, QString> skinResources;
};
