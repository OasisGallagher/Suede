#pragma once
#include <QMap>
#include <QMutex>
#include <QDockWidget>
#include <QTableWidget>

#include "tools/enum.h"
#include "../winbase.h"

BETTER_ENUM_MASK(ConsoleMessageType, int,
	Debug = 1,
	Warning = 2,
	Error = 4
)

class Console : public QDockWidget, public WinSingleton<Console> {
	Q_OBJECT

public:
	Console(QWidget* parent);

public:
	virtual void init(Ui::Editor* ui);
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
