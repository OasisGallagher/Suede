#include <QLineEdit>
#include <QListWidgetItem>

#include "checkablecombobox.h"

#define MAX_COMBOX_ITEMS	(sizeof(uint) * 8 - 1)

#define NONE			"None"
#define MIXED			"Mixed"
#define EVERYTHING		"Everything"

CheckableComboBox::CheckableComboBox(QWidget* parent) : QComboBox(parent), selected_(0) {
	view_ = new QListWidget(this);
	view_->setViewMode(QListWidget::ListMode);

	setModel(view_->model());
	setView(view_);

	setEditable(true);
	lineEdit()->setReadOnly(true);
	lineEdit()->setAlignment(Qt::AlignCenter);

	updateText();
}

void CheckableComboBox::setItems(const QStringList& items) {
	clearAll();
	Q_ASSERT(items.size() > 0 && items.size() < MAX_COMBOX_ITEMS);

	int width = minimumWidth();
	for(uint i = 0; i < items.size(); ++i) {
		const QString& str = items[i];

		width = qMax(width, fontMetrics().width(str));
		QListWidgetItem* item = new QListWidgetItem(view_);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

		view_->insertItem(view_->model()->rowCount(), item);

		QCheckBox* box = new QCheckBox(str, view_);
		box->setChecked(true);
		selected_ |= (1 << i);

		view_->setItemWidget(item, box);
		checkBoxes_.push_back(box);
		
		connect(box, SIGNAL(stateChanged(int)), this, SLOT(onSelectedChanged(int)));
	}

	updateText();
	setFixedWidth(width + 24);
}

void CheckableComboBox::onSelectedChanged(int state) {
	int index = checkBoxes_.indexOf((QCheckBox*)sender());
	Q_ASSERT(index >= 0);

	if (state != 0) {
		selected_ |= (1 << index);
	}
	else {
		selected_ &= ~(1 << index);
	}

	updateText();
	emit selectionChanged(selected_);
}

void CheckableComboBox::updateText() {
	int p = -1;
	uint n = count1Bits(selected_, p);

	if (n == 0) {
		setEditText(tr("None"));
	}
	else if (n == 1) {
		setEditText(checkBoxes_[p]->text());
	}
	else if (n == checkBoxes_.size()) {
		setEditText(tr("Everything"));
	}
	else {
		setEditText(tr("Mixed"));
	}
}

uint CheckableComboBox::count1Bits(uint x, int& p) {
	uint n = 0;
	for (int i = 0; x != 0; x >>= 1, ++i) {
		if ((x & 1) != 0) {
			++n;
			p = i;
		}
	}

	return n;
}

void CheckableComboBox::clearAll() {
	selected_ = 0;
	view_->clear();
	qDeleteAll(checkBoxes_);
}

int CheckableComboBox::minimumWidth() {
	static int w = 0;
	if (w == 0) {
		w = qMax(w, fontMetrics().width(NONE));
		w = qMax(w, fontMetrics().width(MIXED));
		w = qMax(w, fontMetrics().width(EVERYTHING));
	}

	return w;
}
