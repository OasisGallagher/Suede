#include <QDateTime>
#include <QTreeView>
#include <QKeyEvent>
#include <QFileDialog>

#include "engine.h"
#include "resources.h"
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
	root_ = Engine::GetSubsystem<Scene>()->GetRootTransform();
}

void HierarchyWindow::tick() {
	onGui();
}

void HierarchyWindow::onGui() {
	view_->bind();

	if (GUI::BeginContextMenuWindow("Context Menu")) {
		if (GUI::ContextMenuWindowItem("Create Empty")) {
			(new GameObject())->GetTransform()->SetParent(Engine::GetSubsystem<Scene>()->GetRootTransform());
		}

		if (GUI::ContextMenuWindowItem("Import...")) {
			importGameObject();
		}

		GUI::EndContextMenuWindowItem();
	}

	drawHierarchy(root_, 0);
	view_->unbind();
}

void HierarchyWindow::drawHierarchy(Transform* root, int depth) {
	const float kIndent = 1.f;
	for (int i = 0; i < root->GetChildCount(); ++i) {
		Transform* child = root->GetChildAt(i);
		GameObject* childGameObject = child->GetGameObject();

		Color oldColor = GUI::GetColor();
		if (!childGameObject->GetActiveSelf()) {
			GUI::SetColor(Color(0.6f, 0.6f, 0.6f));
		}

		int clickCount = 0;

		bool open = GUI::BeginTreeNode(childGameObject,
			childGameObject->GetName().c_str(),
			selection_->contains(childGameObject),
			child->GetChildCount() == 0, &clickCount);

		if (GUI::BeginContextMenuSubWindow(childGameObject->GetName().c_str())) {
			if (GUI::ContextMenuSubWindowItem("Focus")) {
				emit focusGameObjectRequested(childGameObject);
			}

			if (GUI::ContextMenuSubWindowItem("Delete")) {
				Engine::GetSubsystem<Scene>()->DestroyGameObject(childGameObject);
				selection_->remove(childGameObject);
			}

			GUI::EndContextMenuSubWindow();
		}

		if (open) {
			if (child->GetChildCount() != 0) {
				drawHierarchy(child, depth + 1);
			}

			GUI::EndTreeNode();
		}

		GUI::SetColor(oldColor);

		if (clickCount == 2) {
			emit focusGameObjectRequested(childGameObject);
		}
		else if (clickCount == 1) {
			updateSelection(childGameObject);
		}
	}

	/*for (; !queue.empty();) {
		auto current = queue.back();
		queue.pop_back();
		GameObject* go = current.first->GetGameObject();


	}*/
}

void HierarchyWindow::updateSelection(GameObject* go) {
	if (ImGui::GetIO().KeyShift) {
		selection_->add(go);
		enableGameObjectOutline(go, true);
	}
	else if (ImGui::GetIO().KeyCtrl) {
		if (selection_->contains(go)) {
			selection_->remove(go);
			enableGameObjectOutline(go, false);
		}
		else {
			selection_->add(go);
			enableGameObjectOutline(go, true);
		}
	}
	else {
		for (GameObject* item : selection_->gameObjects()) {
			enableGameObjectOutline(item, false);
		}

		selection_->clear();

		selection_->add(go);
		enableGameObjectOutline(go, true);
	}
}

void HierarchyWindow::importGameObject() {
	QString path = QFileDialog::getOpenFileName(this, "Open", Resources::modelDirectory, "*.fbx");
	if (!path.isEmpty() && !(path = QDir(Resources::modelDirectory).relativeFilePath(path)).isEmpty()) {
		Engine::GetSubsystem<Scene>()->Import(path.toStdString(), [=](GameObject* go) {
			if (go != nullptr) {
				emit focusGameObjectRequested(go);
			}
		});
	}
}

void HierarchyWindow::enableGameObjectOutline(GameObject* go, bool enable) {
	for (MeshRenderer* renderer : go->GetComponentsInChildren<MeshRenderer>()) {
		for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
			Material* material = renderer->GetSharedMaterial(i);
			int outline = material->FindPass("Outline");
			if (outline < 0) { continue; }

			bool alreadyEnabled = material->IsPassEnabled(outline);
			if (enable && !alreadyEnabled) {
				renderer->GetMaterial(i)->EnablePass(outline);
			}
			else if (alreadyEnabled) {
				renderer->GetMaterial(i)->DisablePass(outline);
			}
		}
	}
}
