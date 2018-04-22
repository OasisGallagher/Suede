#pragma once
#include <QMap>
#include <QDockWidget>
#include <QTableWidget>

#include "windows/controls/childwindow.h"

class Console : public QDockWidget, public ChildWindow {
	Q_OBJECT

public:
	enum MessageType {
		Debug = 1,
		Warning = 2,
		Error = 4,
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

private slots:
	void onClearMessages();
	void onSelectionChanged(uint mask);
	void onSearchTextChanged(const QString& text);

private:
	void filterMessageByType(int mask);
	void filterMessageBySubString(const QString& substr);

	void showMessage(MessageType type, const QString &message);

	const char* messageIconPath(MessageType type);

private:
	uint mask_;
	QString substr_;
	QList<QString> messages_;
};
