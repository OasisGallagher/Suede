#include <QListWidget>

#include "custominspector.h"

CustomInspector::CustomInspector(const QString& title, Object object)
	: QGroupBox(title), target_(object) {
	form_ = new QFormLayout(this);
}

void CustomInspector::resizeGeometryToFit(QListWidget* w) {
	int height = 0;
	for (int i = 0; i < w->count(); ++i) {
		height += w->sizeHintForRow(i);
	}

	w->setFixedHeight(height + 4);
}

QString CustomInspector::formatRowName(const QString& name) const {
	QString answer;
	for (int i = 0; i < name.length(); ++i) {
		if (!answer.isEmpty() && name[i].isUpper()) {
			answer += " ";
		}

		if (i == 0 && name[i].isLower()) {
			answer += name[i].toUpper();
		}
		else {
			answer += name[i];
		}
	}

	return answer + ": ";
}