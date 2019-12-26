#pragma once
#include <QObject>
#include <QtWidgets/QApplication>

#include "viewer.h"

class Editor;

class QtViewer : public QObject, public Viewer {
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

private:
	Editor* editor_;
	QApplication app_;

private:
	static QString defaultSkin;
	static QMap<QString, QString> skinResources;
};
