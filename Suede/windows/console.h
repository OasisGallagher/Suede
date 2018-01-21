#pragma once
#include <QMap>
#include <QDockWidget>
#include <QTableWidget>

#include "childwindow.h"

class Console : public QDockWidget, public ChildWindow {
	Q_OBJECT

public:
	enum MessageType {
		Debug,
		Warning,
		Error,
	};

public:
	static Console* get();

public:
	Console(QWidget* parent);
	~Console();

public:
	virtual void init();

public:
	void addMessage(MessageType type, const QString& message);

private:
	const char* messageIconPath(MessageType type);

private:
	QTableWidget* table_;
	QList<QString> messages_;
};
