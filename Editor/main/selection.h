#pragma once
#include <QList>
#include "gameobject.h"

class Selection {
public:
	GameObject* gameObject();
	const QList<GameObject*>& gameObjects();

	void add(GameObject* go);
	void remove(GameObject* go);
	void clear();
	bool contains(GameObject* go) const { return gameObjects_.contains(go); }

private:
	void enableOutline(GameObject* go, bool enable);

private:
	QList<quint64> timeStamps_;
	QList<GameObject*> gameObjects_;
};
