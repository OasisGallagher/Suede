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

	timer_ = startTimer(2000);
}

Suede::~Suede() {
}

void Suede::setupUI() {
	ui.setupUi(this);

	QWidget* cw = takeCentralWidget();
	cw->deleteLater();

	addDockWidget(Qt::LeftDockWidgetArea, ui.inspector);
	addDockWidget(Qt::RightDockWidgetArea, ui.game);
	addDockWidget(Qt::RightDockWidgetArea, ui.console, Qt::Vertical);
	addDockWidget(Qt::RightDockWidgetArea, ui.hierarchy, Qt::Horizontal);

	Game::get()->setView(ui.gameWidget);
	Console::get()->setView(ui.consoleWidget);
	Inspector::get()->setView(ui.inspectorWidget);
	Hierarchy::get()->setView(ui.hierarchyWidget);
}

void Suede::timerEvent(QTimerEvent *event) {
	if (event->timerId() != timer_) { return; }
	std::vector<Sprite> sprites;
	Hierarchy::get()->update(Engine::get()->world()->GetRootSprite());
	killTimer(timer_);
}

void Suede::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
		case Qt::Key_Escape:
			qApp->quit();
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
