#pragma once
#include <QMap>
#include <QMutex>
#include <QDockWidget>
#include <QTableWidget>

#include "tools/enum.h"
#include "main/childwindow.h"

BETTER_ENUM_MASK(ConsoleMessageType, int,
	Debug = 1,
	Warning = 2,
	Error = 4
)

class ConsoleWindow : public ChildWindow {
	Q_OBJECT
public:
	enum {
		WindowType = ChildWindowType::Console,
	};

public:
	ConsoleWindow(QWidget* parent);
	~ConsoleWindow() {}

public:
	virtual void awake();
	virtual void tick();

public:
	void addMessage(ConsoleMessageType type, const QString& message);

private slots:
	void onClearMessages();
	void onSelectionChanged(int mask);
	void onSearchTextChanged(const QString& text);

private:
	void filterMessageByType(int mask);
	void filterMessageBySubString(const QString& substr);

	void flushMessages();
	void onErrorMessage(const QString& message);
	void showMessage(const QString& encodedMessage);
	void showMessage(ConsoleMessageType type, const QString& message);

	const char* messageIconPath(ConsoleMessageType type);

private:
	uint mask_;
	QString substr_;

	QMutex mutex_;
	QList<QString> messages_;
	QList<QString> messagesToShow_;
};
