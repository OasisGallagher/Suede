#pragma once
namespace Ui {
	class Suede;
}

class WinBase {
public:
	virtual void init(Ui::Suede* ui) { ui_ = ui; }
	virtual void awake() {}
	virtual void tick() {}

protected:
	Ui::Suede* ui_;
};

template <class T>
class WinSingleton : public WinBase {
public:
	WinSingleton() {
		Q_ASSERT(instance_ == nullptr);  
		instance_ = (T*)this;
	}

	virtual ~WinSingleton() { instance_ = nullptr; }

public:
	static T* instance() { return instance_; }

private:
	static T* instance_;
};

template <class T>
T* WinSingleton<T>::instance_ = nullptr;
