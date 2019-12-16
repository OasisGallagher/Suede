#include "consolewindow.h"

#include <QThread>
#include <QSplitter>
#include <QMessageBox>
#include <QHeaderView>

#include <QDebug>
#include "main/editor.h"
#include "debug/debug.h"

#include "screen.h"

#define LOG_PATH		"editor.log"

ConsoleWindow::ConsoleWindow(QWidget* parent) : ChildWindow(parent), flush_(false), logFile_(LOG_PATH) {
	Debug::SetLogger(this);
	logFile_.open(QFile::WriteOnly);
	logStream_.setDevice(&logFile_);
}

ConsoleWindow::~ConsoleWindow() {
	Debug::SetLogger(nullptr);
}

void ConsoleWindow::initUI() {
	ui_->table->horizontalHeader()->setStretchLastSection(true);
	ui_->table->horizontalHeader()->setVisible(false);
	ui_->table->setColumnCount(2);

	connect(ui_->clear, SIGNAL(clicked()), this, SLOT(onClearMessages()));
	connect(ui_->filter, SIGNAL(selectionChanged(int)), this, SLOT(onSelectionChanged(int)));
	connect(ui_->search, SIGNAL(textChanged(const QString&)), this, SLOT(onSearchTextChanged(const QString&)));

	mask_ = -1;
	ui_->filter->setEnums(+ConsoleMessageType::Everything);
}

void ConsoleWindow::tick() {
	if (!messagesToShow_.empty()) {
		flushMessages();
	}

	if (flush_) {
		logStream_.flush();
		flush_ = false;
	}
}

void ConsoleWindow::closeEvent(QCloseEvent *event) {
	Debug::SetLogger(nullptr);
	ChildWindow::closeEvent(event);
}

void ConsoleWindow::onClearMessages() {
	messages_.clear();
	ui_->table->setRowCount(0);
}

void ConsoleWindow::onSelectionChanged(int mask) {
	mask_ = mask;
	filterMessageByType(mask);
}

void ConsoleWindow::onSearchTextChanged(const QString& text) {
	substr_ = text;
	filterMessageBySubString(text);
}

void ConsoleWindow::addMessage(ConsoleMessageType type, const QString& message) {
	logToFile(type, message);

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

void ConsoleWindow::flushMessages() {
	QMutexLocker locker(&mutex_);
	for (QString message : messagesToShow_) {
		showMessage(message);
	}

	messagesToShow_.clear();
}

void ConsoleWindow::filterMessageByType(int mask) {
	ui_->table->setRowCount(0);

	for (QString msg : messages_) {
		int type = msg.at(0).toLatin1() - '0';
		if ((type & mask) != 0) {
			showMessage(ConsoleMessageType(type), msg.right(msg.length() - 1));
		}
	}
}

void ConsoleWindow::filterMessageBySubString(const QString& substr) {
	ui_->table->setRowCount(0);

	for (QString msg : messages_) {
		if (substr.isEmpty() || msg.contains(substr)) {
			int type = msg.at(0).toLatin1() - '0';
			showMessage(ConsoleMessageType(type), msg.right(msg.length() - 1));
		}
	}
}

const char* ConsoleWindow::messageIconPath(ConsoleMessageType type) {
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

void ConsoleWindow::showMessage(const QString& encodedMessage) {
	int type = encodedMessage.at(0).toLatin1() - '0';
	showMessage(ConsoleMessageType(type), encodedMessage.right(encodedMessage.length() - 1));
}

void ConsoleWindow::showMessage(ConsoleMessageType type, const QString& message) {
	int r = ui_->table->rowCount();
	ui_->table->insertRow(r);
	ui_->table->setColumnWidth(0, 24);

	QTableWidgetItem* icon = new QTableWidgetItem(QIcon(messageIconPath(type)), "");
	QTableWidgetItem* text = new QTableWidgetItem(message.left(message.indexOf('\n')));
	ui_->table->setItem(r, 0, icon);
	ui_->table->setItem(r, 1, text);

	if (type == ConsoleMessageType::Error) {
		onErrorMessage(message);
	}
}

void ConsoleWindow::OnLogMessageReceived(LogLevel level, const char* message) {
	ConsoleMessageType type = ConsoleMessageType::Debug;
	switch (level) {
	case LogLevel::Warning:
		type = ConsoleMessageType::Warning;
		break;

	case LogLevel::Error:
		type = ConsoleMessageType::Error;
		break;
	}

	addMessage(type, message);
}

void ConsoleWindow::logToFile(ConsoleMessageType type, const QString& message) {
	logStream_ << QString::asprintf("[%c] %s\n", *type.to_string(), message.toLatin1().data());
	flush_ = true;
}

void ConsoleWindow::onErrorMessage(const QString& message) {
	switch (QMessageBox::critical(this, "", message, QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore)) {
		case QMessageBox::Retry:
			Debug::Break();
			break;
		case QMessageBox::Abort:
			TerminateProcess(GetCurrentProcess(), 0);
			break;
	}
}
