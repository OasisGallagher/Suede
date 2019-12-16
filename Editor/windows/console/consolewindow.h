#pragma once
#include <QMap>
#include <QFile>
#include <QMutex>
#include <QTextStream>
#include <QDockWidget>
#include <QTableWidget>

#include "tools/enum.h"
#include "debug/debug.h"
#include "main/childwindow.h"

BETTER_ENUM_MASK(ConsoleMessageType, int,
	Debug = 1,
	Warning = 2,
	Error = 4
)

class ConsoleWindow : public ChildWindow, public Debug::Logger {
	Q_OBJECT

public:
	enum {
		WindowType = ChildWindowType::Console,
	};

public:
	ConsoleWindow(QWidget* parent);
	~ConsoleWindow();

public:
	virtual void initUI();
	virtual void tick();

protected:
	virtual void closeEvent(QCloseEvent *event);

public:
	void addMessage(ConsoleMessageType type, const QString& message);

private slots:
	void onClearMessages();
	void onSelectionChanged(int mask);
	void onSearchTextChanged(const QString& text);

	void OnLogMessageReceived(LogLevel level, const char* message);

private:
	void filterMessageByType(int mask);
	void filterMessageBySubString(const QString& substr);

	void flushMessages();
	void onErrorMessage(const QString& message);
	void showMessage(const QString& encodedMessage);
	void showMessage(ConsoleMessageType type, const QString& message);

	void logToFile(ConsoleMessageType type, const QString& message);

	const char* messageIconPath(ConsoleMessageType type);

private:
	uint mask_;
	QString substr_;

	bool flush_;
	QFile logFile_;
	QTextStream logStream_;

	QMutex mutex_;
	QList<QString> messages_;
	QList<QString> messagesToShow_;
};
