#include <QSplitter>
#include <QHeaderView>

#include "console.h"
#include "ui_suede.h"

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

void Console::init(Ui::Suede* ui) {
	ChildWindow::init(ui);
	ui_->table = findChild<QTableWidget*>("table");
	ui_->table->horizontalHeader()->setStretchLastSection(true);
	ui_->table->horizontalHeader()->setVisible(false);
	ui_->table->setColumnCount(2);

	//((View*)widget())->setSize(QSize(300, 200));
}

void Console::addMessage(MessageType type, const QString& message) {
	int r = ui_->table->rowCount();
	ui_->table->insertRow(r);
	ui_->table->setRowHeight(r, 20);
	ui_->table->setColumnWidth(0, 24);

	QTableWidgetItem* icon = new QTableWidgetItem(QIcon(messageIconPath(type)), "");
	QTableWidgetItem* text = new QTableWidgetItem(message.left(message.indexOf('\n')));
	ui_->table->setItem(r, 0, icon);
	ui_->table->setItem(r, 1, text);

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
