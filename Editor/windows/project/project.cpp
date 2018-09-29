#include "project.h"
#include "ui_editor.h"

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

Project::Project(QWidget* parent) :QDockWidget(parent) {
}

Project::~Project() {
}

void Project::init(Ui::Editor* ui) {
	WinBase::init(ui);
	ui->splitter->setStretchFactor(0, 1);
	ui->splitter->setStretchFactor(1, 4);

	QFile file(IGNORE_FILE_PATH);
	if (file.open(QFile::ReadOnly | QFile::Text)) {
		builtinEntries_ = QSet<QString>::fromList(QString(file.readAll()).split('\n'));
	}

	ui->directoryTree->setRootPath(ROOT_PATH);

	ui_->address->setText(ROOT_PATH);
	connect(ui_->address, SIGNAL(editingFinished()), this, SLOT(onAddressChanged()));

	ui_->listWidget->setItemDelegate(new CustomItemDelegate(ui_->listWidget));

	connect(ui_->listWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onItemChanged(QListWidgetItem*)));
	connect(ui_->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onItemDoubleClicked(QListWidgetItem*)));
	connect(ui_->listWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onCustomContextMenu()));
	connect(ui_->listWidget->itemDelegate(), SIGNAL(commitData(QWidget*)), this, SLOT(onItemEdited(QWidget*)));

	connect(ui_->directoryTree, SIGNAL(selectionChanged(const QStringList&)), this, SLOT(onSelectionChanged(const QStringList&)));
	connect(ui_->directoryTree, SIGNAL(requestContextMenuOnItems(const QStringList&)), this, SLOT(onCustomContextMenu()));
}

QStringList Project::selectedEntries(QWidget* sender) {
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

void Project::onAddressChanged() {
	if (!ui_->directoryTree->selectDirectory(ui_->address->text())) {
		Debug::LogWarning("invalid path %s.", ui_->address->text().toLatin1().data());

		ui_->address->undo();
	}
}

void Project::onItemEdited(QWidget* widget) {
	CustomItemDelegate* delegate = (CustomItemDelegate*)ui_->listWidget->itemDelegate();
	createEntry(ui_->listWidget->item(delegate->editedItemIndex().row()));
}

void Project::onItemDoubleClicked(QListWidgetItem* item) {
	QFileInfo info = item->data(Qt::UserRole).toString();
	if (info.isDir()) {
		ui_->directoryTree->selectDirectory(item->data(Qt::UserRole).toString());
	}
	else {
		openEntries(QStringList() << info.filePath());
	}
}

void Project::onItemChanged(QListWidgetItem* item) {
	// the signal is sent for everything:
	// inserts, changing colors, checking boxes, and anything else that "changes" the item..
	renameEntry(item);
}

void Project::onCreateFolder(const QStringList& selected) {
	QString folder = folderPath(selected.front());
	createEntryListItem(folder, newFolderName(folder), NEW_FOLDER_MAGIC, QFileIconProvider().icon(QFileIconProvider::Folder));
}

void Project::onCreateEmptyShader(const QStringList& selected) {
	QString folder = folderPath(selected.front());
	createEntryListItem(folder, newShaderName(folder), NEW_EMPTY_SHADER_MAGIC, QFileIconProvider().icon(QFileIconProvider::File));
}

void Project::onCreateImageEffectShader(const QStringList& selected) {
	QString folder = folderPath(selected.front());
	createEntryListItem(folder, newShaderName(folder), NEW_IMAGE_EFFECT_SHADER_MAGIC, QFileIconProvider().icon(QFileIconProvider::File));
}

void Project::onOpenSelected(const QStringList& selected) {
	openEntries(selected);
}

void Project::onDeleteSelected(const QStringList& selected) {
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

void Project::onShowSelectedInExplorer(const QStringList& selected) {
	QStringList args({ "/select,", "" });
	QProcess* process = new QProcess(this);

	for (const QString& path : selected) {
		args.back() = QDir::toNativeSeparators(path);
		process->start("explorer.exe", args);
	}
}

void Project::onCustomContextMenu() {
	QMenu menu;
	QStringList selected = selectedEntries((QWidget*)sender());

	// sub menu: "Create".
	QMenu* create = menu.addMenu("Create"); {
		QAction* createFolder = new QAction("Folder", create);
		connect(createFolder, &QAction::triggered, this, std::bind(&Project::onCreateFolder, this, selected));
		create->addAction(createFolder);

		create->addSeparator();

		QMenu* createShader = create->addMenu("Shader");
		QAction* emptyShader = new QAction("Empty Shader", createShader);
		connect(emptyShader, &QAction::triggered, this, std::bind(&Project::onCreateEmptyShader, this, selected));

		QAction* imageEffectShader = new QAction("Image Effect Shader", createShader);
		connect(imageEffectShader, &QAction::triggered, this, std::bind(&Project::onCreateImageEffectShader, this, selected));

		createShader->addAction(emptyShader);
		createShader->addAction(imageEffectShader);
	}

	QAction* showInExplorer = new QAction("Show In Explorer", &menu);
	connect(showInExplorer, &QAction::triggered, this, std::bind(&Project::onShowSelectedInExplorer, this, selected));

	QAction* open = new QAction("Open", &menu);
	connect(open, &QAction::triggered, this, std::bind(&Project::onOpenSelected, this, selected));

	QAction* del = new QAction("Delete", &menu);
	connect(del, &QAction::triggered, this, std::bind(&Project::onDeleteSelected, this, selected));
	if (hasBuiltinEntry(selected)) {
		del->setEnabled(false);
	}

	menu.addAction(showInExplorer);
	menu.addAction(open);
	menu.addAction(del);
	menu.exec(QCursor::pos());
}

void Project::onSelectionChanged(const QStringList& directories) {
	if (!directories.empty()) {
		ui_->address->setText(directories.size() > 1 ? "Showing multiple folders..." : directories.front());
		ui_->address->setReadOnly(directories.size() > 1);
	}

	showDirectortiesContent(directories);
}

void Project::removeEntries(const QStringList& selected) {
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

bool Project::renameEntry(QListWidgetItem* item) {
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

bool Project::createEntry(QListWidgetItem* item) {
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

void Project::openEntries(const QStringList& entries) {
	for (const QString& entry : entries) {
		QDesktopServices::openUrl(QUrl::fromLocalFile(entry));
	}
}

void Project::showDirectortiesContent(const QStringList& directories) {
	ui_->listWidget->clear();

	QSet<QString> set;
	QFileIconProvider ip;
	for (const QString& path : directories) {
		for (const QFileInfo& info : QDir(path).entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
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

void Project::createEmptyShader(const QString& path) {
	createFile(path, "resources/templates/empty_shader.txt");
}

void Project::createImageEffectShader(const QString& path) {
	createFile(path, "resources/templates/image_effect_shader.txt");
}

void Project::createEntryListItem(const QString& folder, const QString& name, const QString& magic, const QIcon& icon) {
	ui_->directoryTree->selectDirectory(folder);

	QListWidgetItem* item = new QListWidgetItem(icon, name);
	item->setFlags(item->flags() | Qt::ItemIsEditable);

	ui_->listWidget->addItem(item);
	ui_->listWidget->sortItems();

	// add magic prefix to indicate the entry type.
	item->setData(Qt::UserRole, magic + folder + "/" + name);
	ui_->listWidget->editItem(item);
}

void Project::createFile(const QString &path, const QString& templatePath) {
	if (!QFile::copy(templatePath, path)) {
		Debug::LogError("failed to create %s.", path.toLatin1().data());
	}
}

QString Project::folderPath(const QString& path) {
	QFileInfo info(path);
	return info.isDir() ? path : info.dir().path();
}

QSet<QString> Project::entriesInFolder(const QString& folder, QDir::Filters filter) {
	return QSet<QString>::fromList(QDir(folder).entryList(filter));
}

bool Project::hasBuiltinEntry(const QStringList& selected) {
	for (const QString& item : selected) {
		if (builtinEntries_.contains(item)) {
			return true;
		}
	}

	return false;
}

QString Project::newShaderName(const QString& folder) {
	return newEntryName("New Shader", ".shader", entriesInFolder(folder, QDir::Files));
}

QString Project::newFolderName(const QString& folder) {
	return newEntryName("New Folder", "", entriesInFolder(folder, QDir::Dirs | QDir::NoDotAndDotDot));
}

QString Project::newEntryName(const QString& base, const QString& postfix, const QSet<QString>& used) {
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
