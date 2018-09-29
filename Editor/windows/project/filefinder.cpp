#include "filefinder.h"

#include <QThreadPool>
#include <QDirIterator>

class Job : public QRunnable {
public:
	Job(FileFinder* finder, const QStringList& items, const QString& pattern) : finder_(finder), items_(items), pattern_(pattern) { }

public:
	virtual void run() {
		findFiles();
		finder_->onFindFinished(pattern_, result_);
	}

private:
	void findFiles() {
	}

private:
	QString pattern_;
	QStringList result_;
	FileFinder* finder_;
	const QStringList& items_;
};

FileFinder::FileFinder(const QString& root, QObject* parent) :QObject(parent), watcher_(this) {
	watcher_.addPath(root);
	connect(&watcher_, SIGNAL(directoryChanged(const QString&)), this, SLOT(onDirectoryChanged(const QString&)));

	QStringList filter;
	QStringList files;
	for (QDirIterator it(root, filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories); it.hasNext();) {
		files << it.next();
		if (it.fileInfo().isDir()) {
			watcher_.addPath(it.path());
		}
	}
}

bool FileFinder::find(const QString& pattern) {
	return false;
}

void FileFinder::onFindFinished(const QString& pattern, const QStringList& result) {

}

void FileFinder::onDirectoryChanged(const QString& path) {

}
