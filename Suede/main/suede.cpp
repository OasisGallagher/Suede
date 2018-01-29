#include <QMenuBar>
#include <QSettings>
#include <QKeyEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

#include "suede.h"
#include "driver.h"
#include "camera.h"

#define LAYOUT_PATH		"resources/settings/layout.ini"

Suede::Suede(QWidget *parent)
	: QMainWindow(parent) {

	Debug::SetLogReceiver(this);

	setupUI();

	QMenu* fileMenu = menuBar()->findChild<QMenu*>("file");
	QList<QAction*> actions = fileMenu->actions();

	connect(actions[0], SIGNAL(triggered()), this, SLOT(screenCapture()));
	connect(actions[1], SIGNAL(triggered()), qApp, SLOT(quit()));

	QMenu* helpMenu = menuBar()->findChild<QMenu*>("help");
	actions = helpMenu->actions();
	connect(actions[0], SIGNAL(triggered()), this, SLOT(aboutBox()));
}

Suede::~Suede() {
	Debug::SetLogReceiver(nullptr);
}

void Suede::setupUI() {
	ui.setupUi(this);

	QWidget* cw = takeCentralWidget();
	delete cw;

	setDockNestingEnabled(true);

	childWindows_[ChildWindowGame] = Game::get();
	childWindows_[ChildWindowConsole] = Console::get();
	childWindows_[ChildWindowInspector] = Inspector::get();
	childWindows_[ChildWindowHierarchy] = Hierarchy::get();

	QSettings settings(LAYOUT_PATH, QSettings::IniFormat);
	QByteArray state = settings.value("State").toByteArray();
	QByteArray geom = settings.value("Geometry").toByteArray();

	if (state.isEmpty() || geom.isEmpty()) {
		addDockWidget(Qt::TopDockWidgetArea, Game::get());
		splitDockWidget(Hierarchy::get(), Game::get(), Qt::Horizontal);
		splitDockWidget(Game::get(), Inspector::get(), Qt::Horizontal);
		splitDockWidget(Game::get(), Console::get(), Qt::Vertical);
	}
	else {
		restoreGeometry(geom);
		restoreState(state, 3350);
	}
}

void Suede::awake() {
	show();

	for (int i = ChildWindowGame; i < ChildWindowCount; ++i) {
		dynamic_cast<ChildWindow*>(childWindows_[i])->init(&ui);
	}

	for (int i = ChildWindowGame; i < ChildWindowCount; ++i) {
		dynamic_cast<ChildWindow*>(childWindows_[i])->awake();
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

void Suede::closeEvent(QCloseEvent *event) {
	QSettings settings(LAYOUT_PATH, QSettings::IniFormat);
	settings.setValue("State", saveState(3350));
	settings.setValue("Geometry", saveGeometry());

	QMainWindow::closeEvent(event);
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
				if (index < ChildWindowCount) {
					showChildWindow(index, !childWindowVisible(index));
				}
			}
			break;
	}
}

void Suede::aboutBox() {
	QMessageBox::information(this, "Suede", QString::asprintf(
		"Vendor: %s\n"
		"Renderer: %s\n"
		"OpenGL: %s\n"
		"GLSL: %s",
		Driver::GetVendor(), 
		Driver::GetRenderer(), 
		Driver::GetVersion(), 
		Driver::GetGLSLVersion()));
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
			Debug::Break();
			break;
	}
}
