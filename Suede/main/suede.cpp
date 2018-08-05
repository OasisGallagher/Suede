#include <QMenuBar>
#include <QKeyEvent>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

#include "suede.h"
#include "driver.h"
#include "camera.h"

#include "prefs.h"

#include "../widgets/status/status.h"
#include "../windows/lighting/lighting.h"
#include "../widgets/dialogs/aboutdialog.h"
#include "../widgets/dialogs/colorpicker.h"
#include "../widgets/dialogs/preferences.h"

#define LAYOUT_PATH		"resources/settings/layout.ini"

namespace PrefsKeys {
	static QString state("_state");
	static QString geometry("_geometry");

	static int stateVersion = 3350;
}

Suede::Suede(QWidget *parent) : QMainWindow(parent) {
	Debug::SetLogReceiver(this);
	
	setupUI();
	setStatusBar(new Status(this));

	initializeFileMenu();
	initializeEditMenu();
	initializeWindowMenu();
	initializeHelpMenu();
}

Suede::~Suede() {
	delete[] childWindows_;
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

	childWindows_[ChildWindowType::Game] = Game::instance();
	childWindows_[ChildWindowType::Console] = Console::instance();
	childWindows_[ChildWindowType::Inspector] = Inspector::instance();
	childWindows_[ChildWindowType::Hierarchy] = Hierarchy::instance();
	childWindows_[ChildWindowType::Lighting] = Lighting::instance();

	QSettings settings(LAYOUT_PATH, QSettings::IniFormat);
	QByteArray state = Prefs::instance()->load(PrefsKeys::state).toByteArray();
	QByteArray geom = Prefs::instance()->load(PrefsKeys::geometry).toByteArray();
	
	if (state.isEmpty() || geom.isEmpty()) {
		initializeLayout();
	}
	else {
		restoreGeometry(geom);
		restoreState(state, PrefsKeys::stateVersion);
	}
}

void Suede::init() {
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		dynamic_cast<WinBase*>(childWindows_[i])->init(&ui);
	}
}

void Suede::awake() {
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		dynamic_cast<WinBase*>(childWindows_[i])->awake();
	}
}

void Suede::tick() {
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		dynamic_cast<WinBase*>(childWindows_[i])->tick();
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
	Prefs::instance()->save(PrefsKeys::state, saveState(PrefsKeys::stateVersion));
	Prefs::instance()->save(PrefsKeys::geometry, saveGeometry());

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
	}
}

void Suede::onAbout() {
	AboutDialog aboutDialog(this);
	aboutDialog.setWindowTitle("Suede");
	aboutDialog.addInformation("Qt", QT_VERSION_STR);
	aboutDialog.addInformation("Vendor", Driver::GetVendor());
	aboutDialog.addInformation("Renderer", Driver::GetRenderer());
	aboutDialog.addInformation("OpenGL", Driver::GetOpenGLVersion());
	aboutDialog.addInformation("GLSL", Driver::GetGLSLVersion());
	aboutDialog.exec();
}

void Suede::onShowWindowMenu() {
	QList<QAction*> actions = menuBar()->findChild<QMenu*>("window")->actions();
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		actions[i]->setChecked(childWindowVisible((ChildWindowType)i));
	}
}

void Suede::onPreferences() {
	Preferences* preferences = new Preferences(this);
	preferences->show();
}

void Suede::onScreenCapture() {
	Texture2D tex = Camera::GetMain()->Capture();

	QString filter = "*.jpg;;*.png";
	
	QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	QString path = QFileDialog::getSaveFileName(this, "", desktop, filter);
	
	if (path.isEmpty()) {
		return;
	}

	std::vector<uchar> data;
	if (path.endsWith(".jpg")) {
		if (!tex->EncodeToJPG(data)) {
			return;
		}
	}
	else if (path.endsWith(".png")) {
		if (!tex->EncodeToPNG(data)) {
			return;
		}
	}

	QImage image;
	if (image.loadFromData(&data[0], data.size())) {
		image.save(path);
	}
}

void Suede::OnLogMessage(LogLevel level, const char* message) {
	switch (level) {
		case LogLevelDebug:
			Console::instance()->addMessage(ConsoleMessageType::Debug, message);
			break;

		case LogLevelWarning:
			Console::instance()->addMessage(ConsoleMessageType::Warning, message);
			break;

		case LogLevelError:
			Console::instance()->addMessage(ConsoleMessageType::Error, message);
			Debug::Break();
			break;
	}
}

void Suede::initializeFileMenu() {
	QMenu* menu = menuBar()->findChild<QMenu*>("file");
	QList<QAction*> actions = menu->actions();

	actions[0]->setShortcut(Qt::CTRL + Qt::Key_P);
	actions[1]->setShortcut(Qt::ALT + Qt::Key_F4);

	connect(actions[0], SIGNAL(triggered()), this, SLOT(onScreenCapture()));
	connect(actions[1], SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Suede::initializeEditMenu() {
	QMenu* menu = menuBar()->findChild<QMenu*>("edit");
	QList<QAction*> actions = menu->actions();

	connect(actions[0], SIGNAL(triggered()), this, SLOT(onPreferences()));
}

void Suede::initializeWindowMenu() {
	QMenu* menu = menuBar()->findChild<QMenu*>("window");
	connect(menu, SIGNAL(aboutToShow()), this, SLOT(onShowWindowMenu()));

	for (int i = 0; i < ChildWindowType::size(); ++i) {
		QAction* a = new QAction(ChildWindowType::from_int(i).to_string(), menu);
		a->setCheckable(true);
		a->setShortcut(Qt::CTRL + Qt::Key(Qt::Key_1 + i));
		connect(a, SIGNAL(triggered()), this, SLOT(onToggleWindowVisible()));

		menu->addAction(a);
	}
}

void Suede::initializeLayout() {
	addDockWidget(Qt::TopDockWidgetArea, Game::instance());
	splitDockWidget(Hierarchy::instance(), Game::instance(), Qt::Horizontal);
	splitDockWidget(Game::instance(), Inspector::instance(), Qt::Horizontal);
	splitDockWidget(Game::instance(), Console::instance(), Qt::Vertical);

	showChildWindow(ChildWindowType::Lighting, false);
}

void Suede::initializeHelpMenu() {
	QMenu* menu = menuBar()->findChild<QMenu*>("help");
	QList<QAction*> actions = menu->actions();
	connect(actions[0], SIGNAL(triggered()), this, SLOT(onAbout()));
}

void Suede::onToggleWindowVisible() {
	QList<QAction*> actions = menuBar()->findChild<QMenu*>("window")->actions();
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		if (actions[i] == sender()) {
			showChildWindow((ChildWindowType)i, !childWindowVisible((ChildWindowType)i));
			break;
		}
	}
}
