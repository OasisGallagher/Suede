#include <QDateTime>
#include <QTreeView>
#include <QKeyEvent>

#include "engine.h"
#include "main/editor.h"

#include "gui.h"
#include "imgui.h"
#include "scene.h"
#include "hierarchywindow.h"
#include "os/filesystem.h"
#include "main/selection.h"
#include "main/imguiwidget.h"

HierarchyWindow::HierarchyWindow(QWidget* parent) : ChildWindow(parent) {
	selection_ = editor_->selection();
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
			selection_->contains(go),
			childCount == 0, &clickCount)) {
			for (int i = 0; i < childCount; ++i) {
				queue.push_back(std::make_pair(current.first->GetChildAt(i), current.second + 1));
			}

			if (GUI::BeginContextMenu(nullptr)) {
				if (GUI::ContextMenuItem("Focus")) {
					emit focusGameObject(go);
				}

				if (GUI::ContextMenuItem("Delete")) {
					Engine::GetSubsystem<Scene>()->DestroyGameObject(go);
					selection_->remove(go);
				}

				GUI::EndContextMenu();
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
	if (input_->GetKey(KeyCode::Shift)) {
		selection_->add(go);
	}
	else if (input_->GetKey(KeyCode::Ctrl)) {
		if (selection_->contains(go)) {
			selection_->remove(go);
		}
		else {
			selection_->add(go);
		}
	}
	else {
		selection_->clear();
		selection_->add(go);
	}
}
