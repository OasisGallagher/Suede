#include <QMenuBar>
#include <QSplitter>
#include <QKeyEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QtWinExtras/QtWin>

#include "suede.h"
#include "world.h"
#include "engine.h"
#include "camera.h"

#include "windows/game.h"
#include "windows/canvas.h"
#include "windows/console.h"
#include "windows/inspector.h"
#include "windows/hierarchy.h"

Suede::Suede(QWidget *parent)
	: QMainWindow(parent) {

	OS::SetPromptCallback(this);
	Debug::SetLogReceiver(this);

	setupUI();

	QMenu* fileMenu = menuBar()->findChild<QMenu*>("file");
	QList<QAction*> actions = fileMenu->actions();

	connect(actions[0], SIGNAL(triggered()), this, SLOT(screenCapture()));
	connect(actions[1], SIGNAL(triggered()), qApp, SLOT(quit()));
}

Suede::~Suede() {
	Debug::SetLogReceiver(nullptr);
}

void Suede::setupUI() {
	ui.setupUi(this);

	QWidget* cw = takeCentralWidget();
	cw->deleteLater();

	childWindows_[ChildWindowGame] = Game::get();
	childWindows_[ChildWindowConsole] = Console::get();
	childWindows_[ChildWindowInspector] = Inspector::get();
	childWindows_[ChildWindowHierarchy] = Hierarchy::get();

	addDockWidget(Qt::LeftDockWidgetArea, Inspector::get());
	addDockWidget(Qt::RightDockWidgetArea, Game::get());
	addDockWidget(Qt::RightDockWidgetArea, Console::get(), Qt::Vertical);
	addDockWidget(Qt::RightDockWidgetArea, Hierarchy::get(), Qt::Horizontal);

	const QRect& r = Inspector::get()->geometry();
	Inspector::get()->setGeometry(r.x(), r.y(), 40, r.height());

	//menuBar()->hide();
	for (int i = ChildWindowGame + 1; i < ChildWindowCount; ++i) {
		showChildWindow(i, false);
	}

	//showChildWindow(ChildWindowConsole, true);

	for (int i = ChildWindowGame; i < ChildWindowCount; ++i) {
		dynamic_cast<ChildWindow*>(childWindows_[i])->ready();
	}
}

void Suede::showChildWindow(int index, bool show) {
	Q_ASSERT(index > 0 && index < ChildWindowCount);
	childWindows_[index]->setVisible(show);
}

bool Suede::childWindowVisible(int index) {
	Q_ASSERT(index > 0 && index < ChildWindowCount);
	return childWindows_[index]->isVisible();
}

void Suede::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
		case Qt::Key_Escape:
			qApp->quit();
			break;

		case Qt::Key_0:
			menuBar()->setVisible(!menuBar()->isVisible());
			break;

		case Qt::Key_1:
		case Qt::Key_2:
		case Qt::Key_3:
		case Qt::Key_4:
		case Qt::Key_5:
		case Qt::Key_6:
		case Qt::Key_7:
		case Qt::Key_8:
		case Qt::Key_9:
			if ((event->modifiers() & Qt::ControlModifier) != 0) {
				int index = event->key() - Qt::Key_0;
				showChildWindow(index, !childWindowVisible(index));
			}
			break;
	}
}

void Suede::screenCapture() {
	std::vector<Sprite> sprites;
	if (!WorldInstance()->GetSprites(ObjectTypeCamera, sprites)) {
		return;
	}

	Camera camera = dsp_cast<Camera>(sprites.front());

	Texture2D tex = camera->Capture();

	QString filter = "*.jpg;;*.png";
	
	QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	QString path = QFileDialog::getSaveFileName(this, "", desktop, filter);
	
	if (path.isEmpty()) {
		return;
	}

	std::vector<uchar> data;
	if (path.endsWith(".jpg") && !tex->EncodeToJpg(data)) {
		return;
	}

	if (path.endsWith(".png") && !tex->EncodeToPng(data)) {
		return;
	}

	QImage image;
	if (image.loadFromData(&data[0], data.size())) {
		image.save(path);
	}
}

bool Suede::OnPrompt(const char* message) {
	QMessageBox::StandardButtons buttons = (QMessageBox::Yes | QMessageBox::No);

	QMessageBox box(QMessageBox::Question, tr("Question"), QString(message), buttons, this);
	box.setButtonText(QMessageBox::Yes, tr("Yes"));
	box.setButtonText(QMessageBox::No, tr("No"));

	return box.exec() == (int)QMessageBox::Yes;
}

void Suede::OnLogMessage(LogLevel level, const char* message) {
	switch (level) {
		case LogLevelDebug:
			Console::get()->addMessage(Console::Debug, message);
			break;

		case LogLevelWarning:
			Console::get()->addMessage(Console::Warning, message);
			break;

		case LogLevelError:
			Console::get()->addMessage(Console::Error, message);
			__debugbreak();
			break;
	}
}
