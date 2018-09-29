#pragma once
#include <QObject>
#include <QFileSystemWatcher>

class FileFinder : QObject {
	Q_OBJECT

public:
	FileFinder(const QString& root, QObject* parent = Q_NULLPTR);

public:
	bool find(const QString& pattern);

public:
	void onFindFinished(const QString& pattern, const QStringList& result);

signals:
	void findFinished(const QString& pattern);

private slots:
	void onDirectoryChanged(const QString& path);

private:
	QFileSystemWatcher watcher_;
};
