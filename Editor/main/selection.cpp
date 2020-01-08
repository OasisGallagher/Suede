#include "selection.h"

#include <QDateTime>

GameObject* Selection::gameObject() {
	auto pos = std::min_element(timeStamps_.begin(), timeStamps_.end());
	return pos != timeStamps_.end() ? gameObjects_[pos - timeStamps_.begin()] : nullptr;
}

const QList<GameObject*>& Selection::gameObjects() {
	return gameObjects_;
}

void Selection::add(GameObject* go) {
	if (!gameObjects_.contains(go)) {
		gameObjects_.push_back(go);
		timeStamps_.push_back(QDateTime::currentMSecsSinceEpoch());
	}
}

void Selection::remove(GameObject* go) {
	int index = gameObjects_.indexOf(go);
	if (index >= 0) {
		gameObjects_.removeAt(index);
		timeStamps_.removeAt(index);
	}
}
