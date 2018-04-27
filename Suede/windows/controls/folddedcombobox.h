#pragma once

#include <QMenu>
#include <QComboBox>

class FolddedComboBox : public QComboBox {
	Q_OBJECT

public:
	FolddedComboBox(QWidget* parent = Q_NULLPTR);

signals:
	void selectionChanged(const QString& path);

public:
	void setItems(const QStringList& items);

protected:
	virtual void showPopup();
	virtual void hidePopup();
};
