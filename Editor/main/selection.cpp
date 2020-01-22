#include "selection.h"

#include <QDateTime>

#include "renderer.h"

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

		enableOutline(go, true);
	}
}

void Selection::remove(GameObject* go) {
	int index = gameObjects_.indexOf(go);
	if (index >= 0) {
		enableOutline(gameObjects_[index], false);

		gameObjects_.removeAt(index);
		timeStamps_.removeAt(index);
	}
}

void Selection::clear() {
	for (GameObject* go : gameObjects_) {
		enableOutline(go, false);
	}

	gameObjects_.clear(); 
	timeStamps_.clear();
}

void Selection::enableOutline(GameObject* go, bool enable) {
	//for (MeshRenderer* renderer : go->GetComponentsInChildren<MeshRenderer>()) {
	//	for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
	//		Material* material = renderer->GetSharedMaterial(i);
	//		int outline = material->FindPass("Outline");
	//		if (outline < 0) { continue; }

	//		// TODO: copy material...
	//		material = renderer->GetMaterial(i);
	//		if (enable) {
	//			material->EnablePass(outline);
	//		}
	//		else {
	//			material->DisablePass(outline);
	//		}
	//	}
	//}
}
