#pragma once
namespace Ui {
	class Suede;
}

class WinBase {
public:
	WinBase() : ui_(nullptr) {}
	virtual ~WinBase() {}

public:
	/**
	 * @brief called on qt ready.
	 */
	virtual void init(Ui::Suede* ui) { ui_ = ui; }

	/**
	 * @brief called on engine ready.
	 */
	virtual void awake() {}

	/**
	 * @brief called once per frame.
	 */
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
