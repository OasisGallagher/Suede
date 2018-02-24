#include <QListWidget>

#include "custominspector.h"

CustomInspector::CustomInspector(const QString& title, Object object) 
	: QGroupBox(title), target_(object) {
	form_ = new QFormLayout(this);
}

void CustomInspector::shrinkToFit(QListWidget* w) {
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

QString CustomInspector::float2QString(float f) {
	QString ans;
	ans.setNum(f, 'f', 1);
	int dot = ans.indexOf('.');
	if (dot >= 0 && ans[dot + 1] == '0') {
		ans.remove(dot, 2);
	}

	return ans;
}
