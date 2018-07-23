#pragma once
#include <QMap>
#include <QDockWidget>
#include <QTableWidget>

#include "tools/enum.h"
#include "../base/childwindow.h"

BETTER_ENUM_MASK(ConsoleMessageType, int,
	Debug = 1,
	Warning = 2,
	Error = 4
)

class Console : public QDockWidget, public ChildWindow {
	Q_OBJECT

public:
	static Console* get();

public:
	Console(QWidget* parent);
	~Console();

public:
	virtual void init(Ui::Suede* ui);

public:
	void addMessage(ConsoleMessageType type, const QString& message);

private slots:
	void onClearMessages();
	void onSelectionChanged(int mask);
	void onSearchTextChanged(const QString& text);

private:
	void filterMessageByType(int mask);
	void filterMessageBySubString(const QString& substr);

	void showMessage(ConsoleMessageType type, const QString &message);

	const char* messageIconPath(ConsoleMessageType type);

private:
	uint mask_;
	QString substr_;
	QList<QString> messages_;
};
