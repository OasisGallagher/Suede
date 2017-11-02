#pragma once
#include <QMap>
#include <QWidget>
#include <QTableWidget>

#include "window.h"

class Console : public Window {
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
	void addMessage(MessageType type, const QString& message);

private:
	Console() { }
	virtual void initialize();

	const char* messageIconPath(MessageType type);

private:
	QTableWidget* table_;
};
