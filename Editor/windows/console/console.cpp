#include <QThread>
#include <QSplitter>
#include <QMessageBox>
#include <QHeaderView>

#include "console.h"
#include "ui_editor.h"
#include "debug/debug.h"

Console::Console(QWidget* parent) : QDockWidget(parent) {
}

void Console::init(Ui::Editor* ui) {
	WinBase::init(ui);
	ui_->table = findChild<QTableWidget*>("table");
	ui_->table->horizontalHeader()->setStretchLastSection(true);
	ui_->table->horizontalHeader()->setVisible(false);
	ui_->table->setColumnCount(2);

	connect(ui_->clear, SIGNAL(clicked()), this, SLOT(onClearMessages()));
	connect(ui_->filter, SIGNAL(selectionChanged(int)), this, SLOT(onSelectionChanged(int)));
	connect(ui_->search, SIGNAL(textChanged(const QString&)), this, SLOT(onSearchTextChanged(const QString&)));

	mask_ = -1;
	ui_->filter->setEnums(+ConsoleMessageType::Everything);
}

void Console::tick() {
	if (!messagesToShow_.empty()) {
		flushMessages();
	}
}

void Console::onClearMessages() {
	messages_.clear();
	ui_->table->setRowCount(0);
}

void Console::onSelectionChanged(int mask) {
	mask_ = mask;
	filterMessageByType(mask);
}

void Console::onSearchTextChanged(const QString& text) {
	substr_ = text;
	filterMessageBySubString(text);
}

void Console::addMessage(ConsoleMessageType type, const QString& message) {
	QString encodedMessage = (type + '0') + message;
	if ((type & mask_) != 0 && (substr_.isEmpty() || message.contains(substr_))) {
		if (QThread::currentThread() != thread()) {
			QMutexLocker locker(&mutex_);
			messagesToShow_.append(encodedMessage);
		}
		else {
			showMessage(type, message);
		}
	}

	messages_.push_back(encodedMessage);
}

void Console::flushMessages() {
	QMutexLocker locker(&mutex_);
	for (QString message : messagesToShow_) {
		showMessage(message);
	}

	messagesToShow_.clear();
}

void Console::filterMessageByType(int mask) {
	ui_->table->setRowCount(0);

	for (QString msg : messages_) {
		int type = msg.at(0).toLatin1() - '0';
		if ((type & mask) != 0) {
			showMessage(ConsoleMessageType(type), msg.right(msg.length() - 1));
		}
	}
}

void Console::filterMessageBySubString(const QString& substr) {
	ui_->table->setRowCount(0);

	for (QString msg : messages_) {
		if (substr.isEmpty() || msg.contains(substr)) {
			int type = msg.at(0).toLatin1() - '0';
			showMessage(ConsoleMessageType(type), msg.right(msg.length() - 1));
		}
	}
}

const char* Console::messageIconPath(ConsoleMessageType type) {
	const char* path = "";
	switch (type) {
		case ConsoleMessageType::Debug:
			path = ":/images/debug";
			break;
		case ConsoleMessageType::Warning:
			path = ":/images/warning";
			break;
		case ConsoleMessageType::Error:
			path = ":/images/error";
			break;
	}

	return path;
}

void Console::showMessage(const QString& encodedMessage) {
	int type = encodedMessage.at(0).toLatin1() - '0';
	showMessage(ConsoleMessageType(type), encodedMessage.right(encodedMessage.length() - 1));
}

void Console::showMessage(ConsoleMessageType type, const QString& message) {
	int r = ui_->table->rowCount();
	ui_->table->insertRow(r);
	ui_->table->setColumnWidth(0, 24);

	QTableWidgetItem* icon = new QTableWidgetItem(QIcon(messageIconPath(type)), "");
	QTableWidgetItem* text = new QTableWidgetItem(message.left(message.indexOf('\n')));
	ui_->table->setItem(r, 0, icon);
	ui_->table->setItem(r, 1, text);
	ui_->table->scrollToBottom();

	if (type == ConsoleMessageType::Error) {
		onErrorMessage(message);
	}
}

void Console::onErrorMessage(const QString& message) {
	switch (QMessageBox::critical(this, "", message, QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore)) {
		case QMessageBox::Retry:
			Debug::Break();
			break;
		case QMessageBox::Abort:
			TerminateProcess(GetCurrentProcess(), 0);
			break;
	}
}
