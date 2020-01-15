#pragma once
#include <set>

#include "gameobject.h"
#include "main/childwindow.h"

class Input;
class Selection;
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

signals:
	void focusGameObject(GameObject* go);

private:
	void onGui();
	void drawHierarchy(Transform* root, int depth);

	void updateSelection(GameObject* go);
	void importGameObject();

	void enableGameObjectOutline(GameObject* go, bool enable);

private:
	IMGUIWidget* view_;

	Transform* root_;
	Selection* selection_;
};
