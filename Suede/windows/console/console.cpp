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

	connect(ui_->clear, SIGNAL(clicked()), this, SLOT(onClearMessages()));
	connect(ui_->filter, SIGNAL(selectionChanged(uint)), this, SLOT(onSelectionChanged(uint)));
	connect(ui_->search, SIGNAL(textChanged(const QString&)), this, SLOT(onSearchTextChanged(const QString&)));

	mask_ = -1;
	ui_->filter->setItems(QStringList() << "Message" << "Warning" << "Error");
}

void Console::onClearMessages() {
	messages_.clear();
	ui_->table->setRowCount(0);
}

void Console::onSelectionChanged(uint mask) {
	mask_ = mask;
	filterMessageByType(mask);
}

void Console::onSearchTextChanged(const QString& text) {
	substr_ = text;
	filterMessageBySubString(text);
}

void Console::addMessage(MessageType type, const QString& message) {
	if ((type & mask_) != 0 && (substr_.isEmpty() || message.contains(substr_))) {
		showMessage(type, message);
	}

	messages_.push_back((type + '0') + message);
}

void Console::filterMessageByType(int mask) {
	ui_->table->setRowCount(0);

	foreach(QString msg, messages_) {
		int type = msg.at(0).toLatin1() - '0';
		if ((type & mask) != 0) {
			showMessage(MessageType(type), msg.right(msg.length() - 1));
		}
	}
}

void Console::filterMessageBySubString(const QString& substr) {
	ui_->table->setRowCount(0);

	foreach(QString msg, messages_) {
		if (substr.isEmpty() || msg.contains(substr)) {
			int type = msg.at(0).toLatin1() - '0';
			showMessage(MessageType(type), msg.right(msg.length() - 1));
		}
	}
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

void Console::showMessage(MessageType type, const QString &message) {
	int r = ui_->table->rowCount();
	ui_->table->insertRow(r);
	ui_->table->setColumnWidth(0, 24);

	QTableWidgetItem* icon = new QTableWidgetItem(QIcon(messageIconPath(type)), "");
	QTableWidgetItem* text = new QTableWidgetItem(message.left(message.indexOf('\n')));
	ui_->table->setItem(r, 0, icon);
	ui_->table->setItem(r, 1, text);
}
