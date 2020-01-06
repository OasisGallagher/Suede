#include <QDateTime>
#include <QTreeView>
#include <QKeyEvent>

#include "engine.h"
#include "main/editor.h"

#include "gui.h"
#include "scene.h"
#include "hierarchywindow.h"
#include "os/filesystem.h"
#include "main/imguiwidget.h"

HierarchyWindow::HierarchyWindow(QWidget* parent) : ChildWindow(parent) {
}

HierarchyWindow::~HierarchyWindow() {
}

void HierarchyWindow::initUI() {
	view_ = new IMGUIWidget(ui_->hierarchyView, nullptr);
	ui_->hierarchyViewLayout->addWidget(view_);

	view_->setForegroundColor(palette().color(foregroundRole()));
	view_->setBackgroundColor(palette().color(backgroundRole()));
}

void HierarchyWindow::awake() {
	input_ = Engine::GetSubsystem<Input>();
	root_ = Engine::GetSubsystem<Scene>()->GetRootTransform();
}

void HierarchyWindow::tick() {
	onGui();
}

GameObject* HierarchyWindow::selectedGameObject() {
	struct TimeComparer {
		bool operator()(const Selection& lhs, const Selection& rhs) const {
			return lhs.time < rhs.time;
		}
	} static timeComparer;

	auto pos = std::min_element(selection_.begin(), selection_.end(), timeComparer);
	return pos != selection_.end() ? pos->go : nullptr;
}

QList<GameObject*> HierarchyWindow::selectedGameObjects() {
	QList<GameObject*> list;
	for (auto& item : selection_) {
		list.push_back(item.go);
	}

	return list;
}

void HierarchyWindow::setSelectedGameObjects(const QList<GameObject*>& objects) {
	selection_.clear();
	int time = 0;
	for (GameObject* go : objects) {
		selection_.insert(Selection{ go, time++ });
	}
}

void HierarchyWindow::onGui() {
	view_->bind();
	drawHierarchy();
	view_->unbind();
}

void HierarchyWindow::drawHierarchy() {
	std::vector<std::pair<Transform*, int>> queue;
	for (int i = 0; i < root_->GetChildCount(); ++i) {
		queue.push_back(std::make_pair(root_->GetChildAt(i), 0));
	}

	for (; !queue.empty();) {
		auto current = queue.back();
		queue.pop_back();
		GameObject* go = current.first->GetGameObject();

		GUI::Indent(current.second * 10);
		int childCount = current.first->GetChildCount();

		int clickCount = 0;
		Color oldColor = GUI::GetColor();
		if (!go->GetActiveSelf()) {
			GUI::SetColor(Color(0.6f, 0.6f, 0.6f));
		}
		
		if (GUI::BeginTreeNode((void*)(uintptr_t)go->GetInstanceID(),
			go->GetName().c_str(),
			selection_.find(Selection{ go }) != selection_.end(),
			childCount == 0, &clickCount)) {
			for (int i = 0; i < childCount; ++i) {
				queue.push_back(std::make_pair(current.first->GetChildAt(i), current.second + 1));
			}

			GUI::EndTreeNode();
		}

		GUI::Unindent(current.second * 10);
		
		GUI::SetColor(oldColor);

		if (clickCount == 2) {
			emit focusGameObject(go);
		}
		else if (clickCount == 1) {
			updateSelection(go);
		}
	}
}

void HierarchyWindow::updateSelection(GameObject* go) {
	Selection item{ go, QDateTime::currentMSecsSinceEpoch() };

	if (input_->GetKey(KeyCode::Shift)) {
		selection_.insert(item);
	}
	else if (input_->GetKey(KeyCode::Ctrl)) {
		if (selection_.find(item) != selection_.end()) {
			selection_.erase(item);
		}
		else {
			selection_.insert(item);
		}
	}
	else {
		selection_.clear();
		selection_.insert(item);
	}
}
