#pragma once
namespace Ui {
	class Suede;
}

class ChildWindow {
public:
	virtual void init(Ui::Suede* ui);
	virtual void awake() {}

protected:
	Ui::Suede* ui_;
};
