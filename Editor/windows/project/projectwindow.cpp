#include "projectwindow.h"
#include "main/editor.h"
#include "tools/string.h"

#include <functional>

#include <QProcess>
#include <QMessageBox>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QFileIconProvider>
#include <QStyledItemDelegate>

#define NEW_FOLDER_MAGIC					"*d"
#define NEW_EMPTY_SHADER_MAGIC				"*e"
#define NEW_IMAGE_EFFECT_SHADER_MAGIC		"*i"

#define ROOT_PATH							"resources"
#define IGNORE_FILE_PATH					"resources/ignore.txt"

class CustomItemDelegate : public QStyledItemDelegate {
public:
	CustomItemDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

public:
	virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
		index_ = index;
		return QStyledItemDelegate::createEditor(parent, option, index);
	}

public:
	const QModelIndex& editedItemIndex() const { return index_; }

private:
	mutable QModelIndex index_;
};

ProjectWindow::ProjectWindow(QWidget* parent) : ChildWindow(parent) {
	QFile file(IGNORE_FILE_PATH);
	if (file.open(QFile::ReadOnly | QFile::Text)) {
		builtinEntries_ = QSet<QString>::fromList(QString(file.readAll()).split('\n'));
	}
}

ProjectWindow::~ProjectWindow() {
}

QStringList ProjectWindow::selectedEntries(QWidget* sender) {
	QStringList entries;
	if (sender == ui_->listWidget) {
		for (QListWidgetItem* item : ui_->listWidget->selectedItems()) {
			entries.push_back(item->data(Qt::UserRole).toString());
		}
	}
	
	if (entries.empty() || sender == ui_->directoryTree) {
		entries = ui_->directoryTree->selectedDirectories();
	}

	return entries;
}

void ProjectWindow::initUI() {
	tree_.Create(ROOT_PATH, ".*");

	ui_->splitter->setStretchFactor(0, 1);
	ui_->splitter->setStretchFactor(1, 4);

	ui_->directoryTree->setRootPath(ROOT_PATH);

	ui_->address->setText(ROOT_PATH);
	connect(ui_->address, SIGNAL(editingFinished()), this, SLOT(onAddressChanged()));

	connect(ui_->searchFile, SIGNAL(textChanged(const QString&)), this, SLOT(onFindFileFieldChanged()));

	ui_->listWidget->setItemDelegate(new CustomItemDelegate(ui_->listWidget));

	connect(ui_->listWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onItemChanged(QListWidgetItem*)));
	connect(ui_->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onItemDoubleClicked(QListWidgetItem*)));
	connect(ui_->listWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onCustomContextMenu()));
	connect(ui_->listWidget->itemDelegate(), SIGNAL(commitData(QWidget*)), this, SLOT(onItemEdited(QWidget*)));

	connect(ui_->directoryTree, SIGNAL(directoryChanged(const QString&)), this, SLOT(onDirectoryChanged(const QString&)));
	connect(ui_->directoryTree, SIGNAL(contentChanged(const QStringList&)), this, SLOT(onSelectedDirectoriesChanged(const QStringList&)));
	connect(ui_->directoryTree, SIGNAL(selectionChanged(const QStringList&)), this, SLOT(onSelectedDirectoriesChanged(const QStringList&)));
	connect(ui_->directoryTree, SIGNAL(requestContextMenuOnItems(const QStringList&)), this, SLOT(onCustomContextMenu()));

	ui_->directoryTree->selectDirectory("resources");
}

void ProjectWindow::onAddressChanged() {
	if (!ui_->directoryTree->selectDirectory(ui_->address->text())) {
		Debug::LogWarning("invalid path %s.", ui_->address->text().toLatin1().data());

		ui_->address->undo();
	}
}

void ProjectWindow::onFindFileFieldChanged() {
	reloadFindResults();
}

void ProjectWindow::onItemEdited(QWidget* widget) {
	CustomItemDelegate* delegate = (CustomItemDelegate*)ui_->listWidget->itemDelegate();
	createEntry(ui_->listWidget->item(delegate->editedItemIndex().row()));
}

void ProjectWindow::onItemDoubleClicked(QListWidgetItem* item) {
	QFileInfo info = item->data(Qt::UserRole).toString();
	if (info.isDir()) {
		ui_->directoryTree->selectDirectory(item->data(Qt::UserRole).toString());
	}
	else {
		openEntries(QStringList() << info.filePath());
	}
}

void ProjectWindow::onItemChanged(QListWidgetItem* item) {
	// the signal is sent for everything:
	// inserts, changing colors, checking boxes, and anything else that "changes" the item..
	renameEntry(item);
}

void ProjectWindow::onCreateFolder(const QStringList& selected) {
	QString folder = folderPath(selected.front());
	createEntryListItem(folder, newFolderName(folder), NEW_FOLDER_MAGIC, QFileIconProvider().icon(QFileIconProvider::Folder));
}

void ProjectWindow::onCreateEmptyShader(const QStringList& selected) {
	QString folder = folderPath(selected.front());
	createEntryListItem(folder, newShaderName(folder), NEW_EMPTY_SHADER_MAGIC, QFileIconProvider().icon(QFileIconProvider::File));
}

void ProjectWindow::onCreateImageEffectShader(const QStringList& selected) {
	QString folder = folderPath(selected.front());
	createEntryListItem(folder, newShaderName(folder), NEW_IMAGE_EFFECT_SHADER_MAGIC, QFileIconProvider().icon(QFileIconProvider::File));
}

void ProjectWindow::onOpenSelected(const QStringList& selected) {
	openEntries(selected);
}

void ProjectWindow::onDeleteSelected(const QStringList& selected) {
 	QString message("Are you sure to delete selected resources?");

	int limit = 5;
 	for (const QString& path : selected) {
		if (--limit < 0) {
			message += "\n...";
			break;
		}

 		message += "\n" + path;
 	}
 
 	if (QMessageBox::warning(this,
 		"Delete Selected Resources", message, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
		removeEntries(selected);
 	}
}

void ProjectWindow::onShowSelectedInExplorer(const QStringList& selected) {
	QStringList args({ "/select,", "" });
	QProcess* process = new QProcess(this);

	for (const QString& path : selected) {
		args.back() = QDir::toNativeSeparators(path);
		process->start("explorer.exe", args);
	}
}

void ProjectWindow::onCustomContextMenu() {
	QMenu menu;
	QStringList selected = selectedEntries((QWidget*)sender());

	// sub menu: "Create".
	QMenu* create = menu.addMenu("Create"); {
		QAction* createFolder = new QAction("Folder", create);
		connect(createFolder, &QAction::triggered, this, std::bind(&ProjectWindow::onCreateFolder, this, selected));
		create->addAction(createFolder);

		create->addSeparator();

		QMenu* createShader = create->addMenu("Shader");
		QAction* emptyShader = new QAction("Empty Shader", createShader);
		connect(emptyShader, &QAction::triggered, this, std::bind(&ProjectWindow::onCreateEmptyShader, this, selected));

		QAction* imageEffectShader = new QAction("Image Effect Shader", createShader);
		connect(imageEffectShader, &QAction::triggered, this, std::bind(&ProjectWindow::onCreateImageEffectShader, this, selected));

		createShader->addAction(emptyShader);
		createShader->addAction(imageEffectShader);
	}

	QAction* showInExplorer = new QAction("Show In Explorer", &menu);
	connect(showInExplorer, &QAction::triggered, this, std::bind(&ProjectWindow::onShowSelectedInExplorer, this, selected));

	QAction* open = new QAction("Open", &menu);
	connect(open, &QAction::triggered, this, std::bind(&ProjectWindow::onOpenSelected, this, selected));

	QAction* del = new QAction("Delete", &menu);
	connect(del, &QAction::triggered, this, std::bind(&ProjectWindow::onDeleteSelected, this, selected));
	if (hasBuiltinEntry(selected)) {
		del->setEnabled(false);
	}

	menu.addAction(showInExplorer);
	menu.addAction(open);
	menu.addAction(del);
	menu.exec(QCursor::pos());
}

void ProjectWindow::onDirectoryChanged(const QString& directory) {
	tree_.Reload(directory.toStdString(), ".*");
	reloadFindResults();
}

void ProjectWindow::onSelectedDirectoriesChanged(const QStringList& directories) {
	if (!directories.empty()) {
		ui_->address->setText(directories.size() > 1 ? "Showing multiple folders..." : directories.front());
		ui_->address->setReadOnly(directories.size() > 1);
	}

	ui_->searchFile->blockSignals(true);
	ui_->searchFile->clear();
	ui_->searchFile->blockSignals(false);

	showDirectortiesContent(directories);
}

void ProjectWindow::removeEntries(const QStringList& selected) {
	for (const QString& path : selected) {
		QFileInfo info(path);
		if (info.isDir() && QDir(path).removeRecursively()) {
			ui_->directoryTree->selectDirectory(info.dir().path());
		}
		else if(!info.isDir()) {
			QFile::remove(path);
		}
	}
}

bool ProjectWindow::renameEntry(QListWidgetItem* item) {
	QString path = item->data(Qt::UserRole).toString();
	QFileInfo info(path);
	if (info.fileName() == item->text()) {
		return false;
	}

	bool ok = false;
	if (info.isDir()) {
		QString target = info.path() + "/" + item->text();
		ok = QDir().rename(path, target);
	}
	else {
		ok = QFile(path).rename(info.dir().path() + "/" + item->text());
	}

	// revert text changes.
	if (!ok) {
		item->setText(info.fileName());
	}

	return true;
}

bool ProjectWindow::createEntry(QListWidgetItem* item) {
	QString path = item->data(Qt::UserRole).toString();
	if (!path.startsWith("*")) {
		return false;
	}

	QString magic = path.left(2);
	path = path.right(path.length() - 2);
	
	if (magic == NEW_FOLDER_MAGIC) {
		QDir().mkpath(path);
	}
	else if (magic == NEW_EMPTY_SHADER_MAGIC) {
		createEmptyShader(path);
	}
	else if (magic == NEW_IMAGE_EFFECT_SHADER_MAGIC) {
		createImageEffectShader(path);
	}
	else {
		Debug::LogError("invalid magic %s.", magic.toLatin1().data());
	}

	item->setData(Qt::UserRole, path);
	return true;
}

void ProjectWindow::openEntries(const QStringList& entries) {
	for (const QString& entry : entries) {
		QDesktopServices::openUrl(QUrl::fromLocalFile(entry));
	}
}

void ProjectWindow::showFindResult(const QStringList& paths) {
	ui_->listWidget->clear();
	QFileIconProvider ip;

	for (const QString& p : paths) {
		QFileInfo info(p);
		if (p == IGNORE_FILE_PATH) {
			continue;
		}

		QListWidgetItem* item = new QListWidgetItem(ip.icon(info), info.fileName());
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		ui_->listWidget->addItem(item);
		item->setData(Qt::UserRole, info.filePath());
	}
}

void ProjectWindow::showDirectortiesContent(const QStringList& directories) {
	ui_->listWidget->clear();

	QSet<QString> set;
	QFileIconProvider ip;
	for (const QString& path : directories) {
		QFileInfoList& infos = QDir(path).entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
		for (const QFileInfo& info : infos) {
			if (info.filePath() == IGNORE_FILE_PATH) {
				continue;
			}

			if (set.contains(info.filePath())) {
				continue;
			}

			QListWidgetItem* item = new QListWidgetItem(ip.icon(info), info.fileName());
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			ui_->listWidget->addItem(item);
			item->setData(Qt::UserRole, info.filePath());
			set.insert(info.filePath());
		}
	}
}

void ProjectWindow::createEmptyShader(const QString& path) {
	createFile(path, "resources/templates/empty_shader.txt");
}

void ProjectWindow::createImageEffectShader(const QString& path) {
	createFile(path, "resources/templates/image_effect_shader.txt");
}

void ProjectWindow::createEntryListItem(const QString& folder, const QString& name, const QString& magic, const QIcon& icon) {
	ui_->directoryTree->selectDirectory(folder);

	QListWidgetItem* item = new QListWidgetItem(icon, name);
	item->setFlags(item->flags() | Qt::ItemIsEditable);

	ui_->listWidget->addItem(item);
	ui_->listWidget->sortItems();

	// add magic prefix to indicate the entry type.
	item->setData(Qt::UserRole, magic + folder + "/" + name);
	ui_->listWidget->editItem(item);
}

void ProjectWindow::createFile(const QString &path, const QString& templatePath) {
	if (!QFile::copy(templatePath, path)) {
		Debug::LogError("failed to create %s.", path.toLatin1().data());
	}
}

QString ProjectWindow::folderPath(const QString& path) {
	QFileInfo info(path);
	return info.isDir() ? path : info.dir().path();
}

QSet<QString> ProjectWindow::entriesInFolder(const QString& folder, QDir::Filters filter) {
	return QSet<QString>::fromList(QDir(folder).entryList(filter));
}

void ProjectWindow::reloadFindResults() {
	QStringList results;
	QString pattern = ui_->searchFile->text();

	if (pattern.isEmpty()) {
		reloadSelectedDirectoriesContent();
	}
	else {
		BoyerMoor bm(pattern.toLatin1().data(), pattern.length());
		for (const FileEntry* entry : tree_.GetAllEntries()) {
			std::string path = entry->GetPath();
			std::string name = FileSystem::GetFileName(path);
			if (bm.Search(name.c_str(), name.length()) != name.length()) {
				results.push_back(path.c_str());
			}
		}

		showFindResult(results);
	}
}

void ProjectWindow::reloadSelectedDirectoriesContent() {
	onSelectedDirectoriesChanged(ui_->directoryTree->selectedDirectories());
}

bool ProjectWindow::hasBuiltinEntry(const QStringList& selected) {
	for (const QString& item : selected) {
		if (builtinEntries_.contains(item)) {
			return true;
		}
	}

	return false;
}

QString ProjectWindow::newShaderName(const QString& folder) {
	return newEntryName("New Shader", ".shader", entriesInFolder(folder, QDir::Files));
}

QString ProjectWindow::newFolderName(const QString& folder) {
	return newEntryName("New Folder", "", entriesInFolder(folder, QDir::Dirs | QDir::NoDotAndDotDot));
}

QString ProjectWindow::newEntryName(const QString& base, const QString& postfix, const QSet<QString>& used) {
	QString name = base + postfix;
	if (!used.contains(name)) { return name; }

	for (int i = 1; ; ++i) {
		name = base + " " + QString::number(i) + postfix;
		if (!used.contains(name)) {
			return name;
		}
	}

	return "";
}
