#pragma once
#include <set>

#include "gameobject.h"
#include "main/childwindow.h"

class Input;
class IMGUIWidget;
class HierarchyWindow : public ChildWindow {
	Q_OBJECT

public:
	enum {
		WindowType = ChildWindowType::Hierarchy,
	};

public:
	HierarchyWindow(QWidget* parent);
	~HierarchyWindow();

public:
	virtual void initUI();
	virtual void awake();
	virtual void tick();

public:
	GameObject* selectedGameObject();
	QList<GameObject*> selectedGameObjects();
	void setSelectedGameObjects(const QList<GameObject*>& objects);

signals:
	void focusGameObject(GameObject* go);

private:
	void onGui();
	void drawHierarchy();

	void updateSelection(GameObject* go);

private:
	IMGUIWidget* view_;

	Input* input_;
	Transform* root_;

	struct Selection {
		bool operator<(const Selection& other) const {
			return go < other.go;
		}

		GameObject* go;
		qint64 time;
	};

	std::set<Selection> selection_;
};
