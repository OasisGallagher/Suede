#include <QMenuBar>
#include <QSplitter>
#include <QKeyEvent>
#include <QFileDialog>

#include "suede.h"

#include "views/game.h"
#include "views/canvas.h"
#include "views/console.h"
#include "views/inspector.h"
#include "views/hierarchy.h"

#include "camera.h"

Suede::Suede(QWidget *parent)
	: QMainWindow(parent) {
	Engine::get()->setLogReceiver(this);

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

	menuBar()->hide();
	for (int i = ChildWindowGame + 1; i < ChildWindowCount; ++i) {
		showChildWindow(i, false);
	}

	Game::get()->setView(ui.gameWidget);
	Console::get()->setView(ui.consoleWidget);
	Inspector::get()->setView(ui.inspectorWidget);
	Hierarchy::get()->setView(ui.hierarchyWidget);
}

void Suede::showChildWindow(int index, bool show) {
	Q_ASSERT(index > 0 && index < ChildWindowCount);
	dockWidgets_[index]->setVisible(show);

	if (index == ChildWindowHierarchy && dockWidgets_[index]->isVisible()) {
		Hierarchy::get()->update(Engine::get()->world()->GetRootSprite());
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
	if (!Engine::get()->world()->GetSprites(ObjectTypeCamera, sprites)) {
		return;
	}

	Camera camera = dsp_cast<Camera>(sprites.front());

	Texture2D tex = camera->Capture();
	std::vector<unsigned char> data;
	if (!tex->EncodeToJpg(data)) {
		return;
	}

	QImage image;
	if (image.loadFromData(&data[0], data.size())) {
		QString filter = "image(*.jpg)";
		QString path = QFileDialog::getSaveFileName(this, "", "", filter);
		if (!path.isEmpty()) {
			image.save(path);
		}
	}
}

void Suede::OnEngineLogMessage(int type, const char* message) {
	if (!childWindowVisible(ChildWindowConsole)) {
		return;
	}

	switch (type) {
		case 0:
			Console::get()->addMessage(Console::Debug, message);
			break;

		case 1:
			Console::get()->addMessage(Console::Warning, message);
			break;

		case 2:
		case 3:
			Console::get()->addMessage(Console::Error, message);
			if (type == 3) { qFatal(message); }
			break;
	}
}
