#include <QMenuBar>
#include <QToolBar>
#include <QKeyEvent>
#include <QStatusBar>
#include <QFileDialog>
#include <QStandardPaths>

#include "editor.h"
#include "camera.h"

#include "prefs.h"

#include "selection.h"
#include "childwindow.h"
#include "../widgets/status/statusbar.h"

#include "../widgets/dialogs/aboutdialog.h"
#include "../widgets/dialogs/colorpicker.h"
#include "../widgets/dialogs/preferences.h"

#define LAYOUT_PATH		"resources/settings/layout.ini"

namespace PrefsKeys {
	static QString state("_state");
	static QString geometry("_geometry");

	static int stateVersion = 3350;
}

Editor::Editor(QWidget *parent) : QMainWindow(parent), preferences_(nullptr), selection_(new Selection()) {
	setupUI();
	setStatusBar(new StatusBar(this));
	
	initializeToolBar();

	initializeFileMenu();
	initializeEditMenu();
	initializeWindowMenu();
	initializeHelpMenu();
}

Editor::~Editor() {
	delete selection_;
	delete[] childWindows_;
	ColorPicker::destroy();
}

void Editor::setupUI() {
	ui_.setupUi(this);

	QWidget* cw = takeCentralWidget();
	delete cw;

	statusBar()->setStatusTip("Ready");

	setDockNestingEnabled(true);

	childWindows_ = new ChildWindow*[ChildWindowType::size()];

	childWindows_[ChildWindowType::Game] = ui_.game;
	childWindows_[ChildWindowType::Console] = ui_.console;
	childWindows_[ChildWindowType::Project] = ui_.project;
	childWindows_[ChildWindowType::Inspector] = ui_.inspector;
	childWindows_[ChildWindowType::Hierarchy] = ui_.hierarchy;
	childWindows_[ChildWindowType::Lighting] = ui_.lighting;

	QByteArray state = Prefs::load(PrefsKeys::state).toByteArray();
	QByteArray geom = Prefs::load(PrefsKeys::geometry).toByteArray();
	
	if (state.isEmpty() || geom.isEmpty()) {
		initializeLayout();
	}
	else {
		restoreGeometry(geom);
		restoreState(state, PrefsKeys::stateVersion);
	}

	for (int i = 0; i < ChildWindowType::size(); ++i) {
		childWindows_[i]->initUI();
	}
}

void Editor::awake() {
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		childWindows_[i]->awake();
	}
}

void Editor::tick() {
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		childWindows_[i]->tick();
	}
}

void Editor::closeEvent(QCloseEvent *event) {
	Prefs::save(PrefsKeys::state, saveState(PrefsKeys::stateVersion));
	Prefs::save(PrefsKeys::geometry, saveGeometry());

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
	//aboutDialog.addInformation("Vendor", OpenGLDriver::GetVendor());
	//aboutDialog.addInformation("Renderer", OpenGLDriver::GetRenderer());
	//aboutDialog.addInformation("OpenGL", OpenGLDriver::GetOpenGLVersion());
	//aboutDialog.addInformation("GLSL", OpenGLDriver::GetGLSLVersion());
	aboutDialog.exec();
}

void Editor::onTogglePlay() {
	playing_ = !playing_;
	playAction_->setIcon(QIcon(QString(":/images/") + (playing_ ? "pause" : "play")));
}

void Editor::onShowWindowMenu() {
	QList<QAction*> actions = menuBar()->findChild<QMenu*>("window")->actions();
	for (int i = 0; i < ChildWindowType::size(); ++i) {
		actions[i]->setChecked(childWindows_[i]->isVisible());
	}
}

void Editor::onPreferences() {
	if (preferences_ == nullptr) {
		preferences_ = new Preferences(this);
	}

	preferences_->show();
}

void Editor::onScreenCapture() {
	ref_ptr<Texture2D> tex = Camera::GetMain()->Capture();

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
	addDockWidget(Qt::TopDockWidgetArea, ui_.game);
	splitDockWidget(ui_.hierarchy, ui_.game, Qt::Horizontal);
	splitDockWidget(ui_.game, ui_.inspector, Qt::Horizontal);
	splitDockWidget(ui_.game, ui_.console, Qt::Vertical);
	tabifyDockWidget(ui_.console, ui_.project);

	childWindow<LightingWindow>()->setVisible(false);
}

void Editor::initializeToolBar() {
	QWidget* dummy1 = new QWidget(this);
	dummy1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QWidget* dummy2 = new QWidget(this);
	dummy2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QToolBar* tb = new QToolBar();
	tb->setMovable(false);
	tb->setFloatable(false);

	tb->setMaximumHeight(22);

	tb->addWidget(dummy1);
	playAction_ = new QAction(QIcon(":/images/pause"), "", tb);
	connect(playAction_, &QAction::triggered, this, &Editor::onTogglePlay);
	tb->addAction(playAction_);
	tb->addWidget(dummy2);

	addToolBar(Qt::TopToolBarArea, tb);
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
			childWindows_[i]->setVisible(!childWindows_[i]->isVisible());
			break;
		}
	}
}
