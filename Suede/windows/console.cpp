#include "console.h"
#include <QSplitter>
#include <QHeaderView>

static Console* consoleInstance;

Console* Console::get() {
	return consoleInstance;
}

Console::Console(QWidget* parent) : QDockWidget(parent) {
	consoleInstance = this;
}

Console::~Console() {
	consoleInstance = nullptr;
}

void Console::init() {
	table_ = findChild<QTableWidget*>("table", Qt::FindChildrenRecursively);
	table_->horizontalHeader()->setStretchLastSection(true);
	table_->horizontalHeader()->setVisible(false);
	table_->setColumnCount(2);

	//((View*)widget())->setSize(QSize(300, 200));
}

void Console::addMessage(MessageType type, const QString& message) {
	int r = table_->rowCount();
	table_->insertRow(r);
	table_->setRowHeight(r, 20);
	table_->setColumnWidth(0, 24);

	QTableWidgetItem* icon = new QTableWidgetItem(QIcon(messageIconPath(type)), "");
	QTableWidgetItem* text = new QTableWidgetItem(message.left(message.indexOf('\n')));
	table_->setItem(r, 0, icon);
	table_->setItem(r, 1, text);

	messages_.push_back(message);
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
