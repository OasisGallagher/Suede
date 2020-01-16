#pragma once

#include <QMenu>
#include <QComboBox>

class FileTree;
class FileEntry;

class FileTreeField : public QComboBox {
	Q_OBJECT

public:
	FileTreeField(QWidget* parent = Q_NULLPTR);
	~FileTreeField();

public:
	void setDirectory(const QString& path, const QString& selected, const QString& regex);

signals:
	void selectionChanged(const QString& path);

protected:
	virtual void showPopup();
	virtual void hidePopup();
	virtual bool eventFilter(QObject *watched, QEvent *event);

private slots:
	void onHideMenu();
	void onSelectItem();

private:
	void createMenu(FileTree &tree);
	void createSubMenu(QMenu* parent, FileEntry* entry);

private:
	QMenu* menu_;

	QString regex_;
	QString directory_;
};
