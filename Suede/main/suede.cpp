#include <QMenuBar>
#include <QSettings>
#include <QKeyEvent>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

#include "suede.h"
#include "driver.h"
#include "camera.h"

#include "../widgets/status/status.h"
#include "../windows/lighting/lighting.h"
#include "../widgets/dialogs/aboutdialog.h"
#include "../widgets/dialogs/colorpicker.h"

#define LAYOUT_PATH		"resources/settings/layout.ini"

Suede::Suede(QWidget *parent) : QMainWindow(parent) {
	Debug::SetLogReceiver(this);
	
	setupUI();
	setStatusBar(new Status(this));

	initializeFileMenu();
	initializeWindowsMenu();
	initializeHelpMenu();
}

Suede::~Suede() {
	delete childWindows_;
	ColorPicker::destroy();
	Debug::SetLogReceiver(nullptr);
}

void Suede::setupUI() {
	ui.setupUi(this);

	QWidget* cw = takeCentralWidget();
	delete cw;

	statusBar()->setStatusTip("Ready");

	setDockNestingEnabled(true);

	childWindows_ = new QDockWidget*[ChildWindowType::size()];

	childWindows_[ChildWindowType::Game] = Game::get();
	childWindows_[ChildWindowType::Console] = Console::get();
	childWindows_[ChildWindowType::Inspector] = Inspector::get();
	childWindows_[ChildWindowType::Hierarchy] = Hierarchy::get();
	childWindows_[ChildWindowType::Lighting] = Lighting::get();

	QSettings settings(LAYOUT_PATH, QSettings::IniFormat);
	QByteArray state = settings.value("State").toByteArray();
	QByteArray geom = settings.value("Geometry").toByteArray();

	if (state.isEmpty() || geom.isEmpty()) {
		initializeLayout();
	}
	else {
		restoreGeometry(geom);
		restoreState(state, 3350);
	}
}

void Suede::awake() {
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		dynamic_cast<WinBase*>(childWindows_[i])->init(&ui);
	}

	show();

	for (int i = 0; i < ChildWindowType::size(); ++i) {
		dynamic_cast<WinBase*>(childWindows_[i])->awake();
	}
}

void Suede::showChildWindow(ChildWindowType index, bool show) {
	Q_ASSERT(index >= 0 && index < ChildWindowType::size());
	childWindows_[index]->setVisible(show);
}

bool Suede::childWindowVisible(ChildWindowType index) {
	Q_ASSERT(index >= 0 && index < ChildWindowType::size());
	return childWindows_[index]->isVisible();
}

void Suede::closeEvent(QCloseEvent *event) {
	QSettings settings(LAYOUT_PATH, QSettings::IniFormat);
	settings.setValue("State", saveState(3350));
	settings.setValue("Geometry", saveGeometry());
	QMainWindow::closeEvent(event);

	emit aboutToClose();
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
				int index = event->key() - Qt::Key_1;
				if (index < ChildWindowType::size()) {
					showChildWindow((ChildWindowType)index, !childWindowVisible((ChildWindowType)index));
				}
			}
			break;
	}
}

void Suede::aboutBox() {
	AboutDialog aboutDialog(this);
	aboutDialog.setWindowTitle("Suede");
	aboutDialog.addInformation("Qt", QT_VERSION_STR);
	aboutDialog.addInformation("Vendor", Driver::GetVendor());
	aboutDialog.addInformation("Renderer", Driver::GetRenderer());
	aboutDialog.addInformation("OpenGL", Driver::GetOpenGLVersion());
	aboutDialog.addInformation("GLSL", Driver::GetGLSLVersion());
	aboutDialog.exec();
}

void Suede::onShowWindowsMenu() {
	QList<QAction*> actions = menuBar()->findChild<QMenu*>("windows")->actions();
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		actions[i]->setChecked(childWindowVisible((ChildWindowType)i));
	}
}

void Suede::screenCapture() {
	Camera camera = WorldInstance()->GetMainCamera();

	Texture2D tex = camera->Capture();

	QString filter = "*.jpg;;*.png";
	
	QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	QString path = QFileDialog::getSaveFileName(this, "", desktop, filter);
	
	if (path.isEmpty()) {
		return;
	}

	std::vector<uchar> data;
	if (path.endsWith(".jpg") && !tex->EncodeToJPG(data)) {
		return;
	}

	if (path.endsWith(".png") && !tex->EncodeToPNG(data)) {
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
			Console::get()->addMessage(ConsoleMessageType::Debug, message);
			break;

		case LogLevelWarning:
			Console::get()->addMessage(ConsoleMessageType::Warning, message);
			break;

		case LogLevelError:
			Console::get()->addMessage(ConsoleMessageType::Error, message);
			Debug::Break();
			break;
	}
}

void Suede::initializeFileMenu() {
	QMenu* menu = menuBar()->findChild<QMenu*>("file");
	QList<QAction*> actions = menu->actions();

	connect(actions[0], SIGNAL(triggered()), this, SLOT(screenCapture()));
	connect(actions[1], SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Suede::initializeWindowsMenu() {
	QMenu* menu = menuBar()->findChild<QMenu*>("windows");
	connect(menu, SIGNAL(aboutToShow()), this, SLOT(onShowWindowsMenu()));

	QAction* before = menu->actions().front();
	for (int i = ChildWindowType::size() - 1; i >= 0; --i) {
		QAction* a = new QAction(ChildWindowType::from_int(i).to_string(), menu);
		a->setCheckable(true);

		menu->insertAction(before, a);
		connect(a, SIGNAL(triggered()), this, SLOT(onToggleWindowVisible()));
		before = a;
	}

	QList<QAction*> actions = menu->actions();
	// skip seperator.
	connect(actions[ChildWindowType::size() + 1], SIGNAL(triggered()), this, SLOT(onShowEnvironment()));
}

void Suede::initializeLayout() {
	addDockWidget(Qt::TopDockWidgetArea, Game::get());
	splitDockWidget(Hierarchy::get(), Game::get(), Qt::Horizontal);
	splitDockWidget(Game::get(), Inspector::get(), Qt::Horizontal);
	splitDockWidget(Game::get(), Console::get(), Qt::Vertical);

	showChildWindow(ChildWindowType::Lighting, false);
}

void Suede::initializeHelpMenu() {
	QMenu* menu = menuBar()->findChild<QMenu*>("help");
	QList<QAction*> actions = menu->actions();
	connect(actions[0], SIGNAL(triggered()), this, SLOT(aboutBox()));
}

void Suede::onShowEnvironment() {
	
}

void Suede::onToggleWindowVisible() {
	QList<QAction*> actions = menuBar()->findChild<QMenu*>("windows")->actions();
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		if (actions[i] == sender()) {
			showChildWindow((ChildWindowType)i, !childWindowVisible((ChildWindowType)i));
			break;
		}
	}
}
