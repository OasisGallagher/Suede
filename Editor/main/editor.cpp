#include <QMenuBar>
#include <QKeyEvent>
#include <QStatusBar>
#include <QFileDialog>
#include <QStandardPaths>

#include "editor.h"
#include "camera.h"
#include "opengldriver.h"

#include "prefs.h"

#include "../widgets/status/statusbar.h"

#include "../windows/project/project.h"
#include "../windows/lighting/lighting.h"

#include "../widgets/dialogs/aboutdialog.h"
#include "../widgets/dialogs/colorpicker.h"
#include "../widgets/dialogs/preferences.h"

#define LOG_PATH		"editor.log"
#define LAYOUT_PATH		"resources/settings/layout.ini"

namespace PrefsKeys {
	static QString state("_state");
	static QString geometry("_geometry");

	static int stateVersion = 3350;
}

Editor::Editor(QWidget *parent) : QMainWindow(parent), preferences_(nullptr), flush_(false), logFile_(LOG_PATH) {
	Debug::SetLogReceiver(this);

	logFile_.open(QFile::WriteOnly);
	logStream_.setDevice(&logFile_);

	setupUI();
	setStatusBar(new StatusBar(this));

	initializeFileMenu();
	initializeEditMenu();
	initializeWindowMenu();
	initializeHelpMenu();
}

Editor::~Editor() {
	logFile_.close();
	delete[] childWindows_;
	ColorPicker::destroy();
	Debug::SetLogReceiver(nullptr);
}

void Editor::setupUI() {
	ui.setupUi(this);

	QWidget* cw = takeCentralWidget();
	delete cw;

	statusBar()->setStatusTip("Ready");

	setDockNestingEnabled(true);

	childWindows_ = new QDockWidget*[ChildWindowType::size()];

	childWindows_[ChildWindowType::Game] = Game::instance();
	childWindows_[ChildWindowType::Console] = Console::instance();
	childWindows_[ChildWindowType::Project] = Project::instance();
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

void Editor::init() {
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		dynamic_cast<WinBase*>(childWindows_[i])->init(&ui);
	}
}

void Editor::awake() {
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		dynamic_cast<WinBase*>(childWindows_[i])->awake();
	}
}

void Editor::tick() {
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		dynamic_cast<WinBase*>(childWindows_[i])->tick();
	}

	if (flush_) {
		logStream_.flush();
		flush_ = false;
	}
}

void Editor::showChildWindow(ChildWindowType index, bool show) {
	Q_ASSERT(index >= 0 && index < ChildWindowType::size());
	childWindows_[index]->setVisible(show);
}

bool Editor::childWindowVisible(ChildWindowType index) {
	Q_ASSERT(index >= 0 && index < ChildWindowType::size());
	return childWindows_[index]->isVisible();
}

void Editor::closeEvent(QCloseEvent *event) {
	Prefs::instance()->save(PrefsKeys::state, saveState(PrefsKeys::stateVersion));
	Prefs::instance()->save(PrefsKeys::geometry, saveGeometry());

	QMainWindow::closeEvent(event);
	emit aboutToClose();
}

void Editor::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
		case Qt::Key_Escape:
			qApp->quit();
			break;

		case Qt::Key_0:
			menuBar()->setVisible(!menuBar()->isVisible());
			break;
	}
}

void Editor::onAbout() {
	AboutDialog aboutDialog(this);
	aboutDialog.setWindowTitle("Suede");
	aboutDialog.addInformation("Qt", QT_VERSION_STR);
	aboutDialog.addInformation("Vendor", OpenGLDriver::GetVendor());
	aboutDialog.addInformation("Renderer", OpenGLDriver::GetRenderer());
	aboutDialog.addInformation("OpenGL", OpenGLDriver::GetOpenGLVersion());
	aboutDialog.addInformation("GLSL", OpenGLDriver::GetGLSLVersion());
	aboutDialog.exec();
}

void Editor::onShowWindowMenu() {
	QList<QAction*> actions = menuBar()->findChild<QMenu*>("window")->actions();
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		actions[i]->setChecked(childWindowVisible((ChildWindowType)i));
	}
}

void Editor::onPreferences() {
	if (preferences_ == nullptr) {
		preferences_ = new Preferences(this);
	}

	preferences_->show();
}

void Editor::onScreenCapture() {
	Texture2D tex = Camera::main()->Capture();

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

void Editor::OnLogMessage(LogLevel level, const char* message) {
	ConsoleMessageType type;
	switch (level) {
		case LogLevelDebug:
			type = ConsoleMessageType::Debug;
			break;

		case LogLevelWarning:
			type = ConsoleMessageType::Warning;
			break;

		case LogLevelError:
			type = ConsoleMessageType::Error;
			break;
	}

	writeLog(type, message);
	Console::instance()->addMessage(type, message);
}

void Editor::initializeFileMenu() {
	QMenu* menu = menuBar()->findChild<QMenu*>("file");
	QList<QAction*> actions = menu->actions();

	actions[0]->setShortcut(Qt::CTRL + Qt::Key_P);
	actions[1]->setShortcut(Qt::ALT + Qt::Key_F4);

	connect(actions[0], SIGNAL(triggered()), this, SLOT(onScreenCapture()));
	connect(actions[1], SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Editor::initializeEditMenu() {
	QMenu* menu = menuBar()->findChild<QMenu*>("edit");
	QList<QAction*> actions = menu->actions();

	connect(actions[0], SIGNAL(triggered()), this, SLOT(onPreferences()));
}

void Editor::initializeWindowMenu() {
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

void Editor::initializeLayout() {
	addDockWidget(Qt::TopDockWidgetArea, Game::instance());
	splitDockWidget(Hierarchy::instance(), Game::instance(), Qt::Horizontal);
	splitDockWidget(Game::instance(), Inspector::instance(), Qt::Horizontal);
	splitDockWidget(Game::instance(), Console::instance(), Qt::Vertical);
	tabifyDockWidget(Console::instance(), Project::instance());

	showChildWindow(ChildWindowType::Lighting, false);
}

void Editor::initializeHelpMenu() {
	QMenu* menu = menuBar()->findChild<QMenu*>("help");
	QList<QAction*> actions = menu->actions();
	connect(actions[0], SIGNAL(triggered()), this, SLOT(onAbout()));
}

void Editor::onToggleWindowVisible() {
	QList<QAction*> actions = menuBar()->findChild<QMenu*>("window")->actions();
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		if (actions[i] == sender()) {
			showChildWindow((ChildWindowType)i, !childWindowVisible((ChildWindowType)i));
			break;
		}
	}
}

void Editor::writeLog(ConsoleMessageType type, const char* message) {
	logStream_ << QString::asprintf("[%c] %s\n", *type.to_string(), message);
	flush_ = true;
}
