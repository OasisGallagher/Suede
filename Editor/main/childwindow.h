#pragma once

#include <QDockWidget>
#include "tools/enum.h"

BETTER_ENUM(ChildWindowType, int,
	Game,
	Console,
	Project,
	Inspector,
	Hierarchy,
	Lighting
)

class Editor;
namespace Ui { class Editor; }

class ChildWindow : public QDockWidget {
public:
	ChildWindow(QWidget* parent);

public:
	virtual void awake() {}
	virtual void tick() {}

protected:
	Editor* editor_;
	Ui::Editor* ui_;
};
