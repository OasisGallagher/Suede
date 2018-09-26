#include <QLineEdit>
#include <QListWidgetItem>

#include "maskfield.h"

#define NONE_STRING				("None")
#define MIXED_STRING			("Mixed")
#define EVERYTHING_STRING		("Everything")

MaskField::MaskField(QWidget* parent) : QComboBox(parent), selected_(0), everything_(0) {
	view_ = new QListWidget(this);
	view_->setViewMode(QListWidget::ListMode);

	setModel(view_->model());
	setView(view_);

	setEditable(true);
	lineEdit()->setReadOnly(true);
	lineEdit()->setAlignment(Qt::AlignCenter);

	updateText();
}

void MaskField::setItems(const QStringList& items, int mask) {
	clearAll();
	Q_ASSERT(items.size() <= MaxMaskItems);

	updateEverythingMask(items.size());
	int width = fontMetrics().width(MIXED_STRING);
	width = qMax(width, addItem(NONE_STRING));
	width = qMax(width, addItem(EVERYTHING_STRING));

	for(QString item : items) {
		width = qMax(width, addItem(item));
	}

	setSelectedMask(mask);
	setFixedWidth(width + 24);
}

int MaskField::addItem(const QString& str) {
	QListWidgetItem* item = new QListWidgetItem(view_);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

	view_->insertItem(view_->model()->rowCount(), item);

	QCheckBox* box = new QCheckBox(str, view_);

	view_->setItemWidget(item, box);
	checkBoxes_.push_back(box);

	connect(box, SIGNAL(stateChanged(int)), this, SLOT(onSelectedChanged(int)));

	return fontMetrics().width(str);
}

void MaskField::updateEverythingMask(int size) {
	everything_ = -1;

	if (size < MaxMaskItems) {
		everything_ = 0;
		for (int i = 0; i < size; ++i) {
			everything_ |= values_[i];
		}
	}
}

void MaskField::onSelectedChanged(int state) {
	int index = checkBoxes_.indexOf((QCheckBox*)sender());
	Q_ASSERT(index >= 0);

	if (state != 0) {
		addSelectedMask(index);
	}
	else {
		removeSelectedMask(index);
	}

	emit selectionChanged(selected_);
}

void MaskField::setSelectedMask(int value) {
	selected_ = value;
	if (selected_ == everything_) {
		selected_ = -1;
	}
	
	updateText();
	updateCheckBoxes();
}

void MaskField::updateCheckBoxes() {
	for (QCheckBox* box : checkBoxes_) {
		box->blockSignals(true);
	}

	checkBoxes_[0]->setChecked(selected_ == 0);
	checkBoxes_[1]->setChecked(selected_ == -1);

	if (selected_ == 0 || selected_ == -1) {
		for (int i = 2; i < checkBoxes_.size(); ++i) {
			checkBoxes_[i]->setChecked(selected_ == -1);
		}
	}
	else {
		for (int i = 2; i < checkBoxes_.size(); ++i) {
			checkBoxes_[i]->setChecked((selected_ & (values_[i - 2])) != 0);
		}
	}

	for (QCheckBox* box : checkBoxes_) {
		box->blockSignals(false);
	}
}

void MaskField::addSelectedMask(int index) {
	if (index == 0) {
		setSelectedMask(0);
	}
	else if (index == 1) {
		setSelectedMask(-1);
	}
	else {
		setSelectedMask(selected_ | values_[index - 2]);
	}
}

void MaskField::removeSelectedMask(int index) {
	if (index == 0) {
		setSelectedMask(-1);
	}
	else if (index == 1) {
		setSelectedMask(0);
	}
	else {
		if (selected_ == -1) {
			selected_ = everything_;
		}

		setSelectedMask(selected_ & ~(values_[index - 2]));
	}
}

void MaskField::updateText() {
	if (selected_ == -1) {
		setEditText(EVERYTHING_STRING);
		return;
	}
	
	uint n = count1Bits(selected_);

	if (n == 0) {
		setEditText(NONE_STRING);
	}
	else if (n == 1) {
		setEditText(checkBoxes_[values_.indexOf(selected_) + 2]->text());
	}
	else {
		setEditText(MIXED_STRING);
	}
}

uint MaskField::count1Bits(uint x) {
	uint n = 0;
	for (int i = 0, t = x; t != 0; t >>= 1, ++i) {
		if ((t & 1) != 0) {
			++n;
		}
	}

	return n;
}

void MaskField::clearAll() {
	selected_ = 0;
	view_->clear();
	qDeleteAll(checkBoxes_);
}
