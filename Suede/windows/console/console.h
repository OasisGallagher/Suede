#pragma once
#include <QMap>
#include <QDockWidget>
#include <QTableWidget>

#include "../controls/childwindow.h"

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
	virtual void init(Ui::Suede* ui);

public:
	void addMessage(MessageType type, const QString& message);

private:
	const char* messageIconPath(MessageType type);

private:
	QList<QString> messages_;
};
