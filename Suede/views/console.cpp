#include "console.h"
#include <QSplitter>
#include <QHeaderView>

Console* Console::get() {
	static Console instance;
	return &instance;
}

void Console::initialize() {
	table_ = view_->findChild<QTableWidget*>("table", Qt::FindDirectChildrenOnly);
	table_->horizontalHeader()->setStretchLastSection(true);
	table_->horizontalHeader()->setVisible(false);
	table_->setColumnCount(2);
}

void Console::addMessage(MessageType type, const QString& message) {
	int r = table_->rowCount();
	table_->insertRow(r);
	table_->setRowHeight(r, 20);
	table_->setColumnWidth(0, 24);

	QTableWidgetItem* icon = new QTableWidgetItem(QIcon(messageIconPath(type)), "");
	QTableWidgetItem* text = new QTableWidgetItem(message);
	table_->setItem(r, 0, icon);
	table_->setItem(r, 1, text);
	table_->scrollToBottom();
}

const char* Console::messageIconPath(MessageType type) {
	const char* path = "";
	switch (type) {
		case Debug:
			path = ":/images/debug";
			break;
		case Warning:
			path = ":/images/warning";
			break;
		case Error:
			path = ":/images/error";
			break;
	}

	return path;
}