#include <QMenuBar>
#include <QSplitter>
#include <QKeyEvent>
#include <QFileDialog>
#include <QStandardPaths>

#include "suede.h"
#include "world.h"
#include "engine.h"
#include "camera.h"

#include "views/game.h"
#include "views/canvas.h"
#include "views/console.h"
#include "views/inspector.h"
#include "views/hierarchy.h"

Suede::Suede(QWidget *parent)
	: QMainWindow(parent) {

	Debug::SetLogReceiver(this);

	setupUI();

	QMenu* fileMenu = menuBar()->findChild<QMenu*>("file");
	QList<QAction*> actions = fileMenu->actions();

	connect(actions[0], SIGNAL(triggered()), this, SLOT(screenCapture()));
	connect(actions[1], SIGNAL(triggered()), qApp, SLOT(quit()));
}

Suede::~Suede() {
}

void Suede::setupUI() {
	ui.setupUi(this);

	QWidget* cw = takeCentralWidget();
	cw->deleteLater();

	dockWidgets_[ChildWindowGame] = ui.game;
	dockWidgets_[ChildWindowConsole] = ui.console;
	dockWidgets_[ChildWindowInspector] = ui.inspector;
	dockWidgets_[ChildWindowHierarchy] = ui.hierarchy;

	addDockWidget(Qt::LeftDockWidgetArea, ui.inspector);
	addDockWidget(Qt::RightDockWidgetArea, ui.game);
	addDockWidget(Qt::RightDockWidgetArea, ui.console, Qt::Vertical);
	addDockWidget(Qt::RightDockWidgetArea, ui.hierarchy, Qt::Horizontal);

	//menuBar()->hide();
	for (int i = ChildWindowGame + 1; i < ChildWindowCount; ++i) {
		showChildWindow(i, false);
	}

	showChildWindow(ChildWindowConsole, true);

	Game::get()->setView(ui.gameWidget);
	Console::get()->setView(ui.consoleWidget);
	Inspector::get()->setView(ui.inspectorWidget);
	Hierarchy::get()->setView(ui.hierarchyWidget);
}

void Suede::showChildWindow(int index, bool show) {
	Q_ASSERT(index > 0 && index < ChildWindowCount);
	dockWidgets_[index]->setVisible(show);

	if (index == ChildWindowHierarchy && dockWidgets_[index]->isVisible()) {
		Hierarchy::get()->update(WorldInstance()->GetRootSprite());
	}
}

bool Suede::childWindowVisible(int index) {
	Q_ASSERT(index > 0 && index < ChildWindowCount);
	return dockWidgets_[index]->isVisible();
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
	
	// TODO: save to desktop...
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

void Suede::OnLogMessage(LogLevel level, const char* message) {
	if (!childWindowVisible(ChildWindowConsole)) {
		return;
	}

	switch (level) {
		case LogLevelDebug:
			Console::get()->addMessage(Console::Debug, message);
			break;

		case LogLevelWarning:
			Console::get()->addMessage(Console::Warning, message);
			break;

		case LogLevelError:
			Console::get()->addMessage(Console::Error, message);
			qFatal(message);
			break;
	}
}
