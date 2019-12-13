#pragma once
// https://stackoverflow.com/a/35856994/2705388

#include <memory>
#include <algorithm>

// an event holds a vector of subscribers_
// when it fires, each is called

template<class... Args>
class _SubscriberBase {
public:
	virtual int order() = 0;
	virtual void* callee() = 0;
	virtual void call(Args... args) = 0;
	virtual bool instanceof(void* t) = 0;
	virtual ~_SubscriberBase() {}
};

template<class T, class... Args>
class _Subscriber : public _SubscriberBase<Args...> {
public:
	typedef T callee_type;

private:
	T* t;
	int o;
	void(T::*f)(Args...);

public:
	_Subscriber(T* _t, void(T::*_f)(Args...), int _order = 0) : t(_t), f(_f), o(_order) {}
	~_Subscriber() {}
	int order() { return o; }
	void* callee() { return t; }
	void call(Args... args)   final { (t->*f)(args...); }
	bool instanceof(void* _t) final { return _t == (void*)t; }
};

// our Listener will derive from EventListener<Listener>
// which holds a list of a events it is subscribed to.
// As these events will have different sigs, we need a base-class.
// We will store pointers to this base-class.
class _EventBase {
public:
	virtual void unsubscribe(void* t) = 0;
};

template<class... Args>
class event : public _EventBase {
public:
	using smart_ptr_type = std::shared_ptr<_SubscriberBase<Args...>>;
	void fire(Args... args) {
		firing_ = true;
		int size = subscribers_.size();
		for (int i = 0; i < size; ++i) {
			subscribers_[i]->call(args...);
		}

		firing_ = false;

		for (auto& t : to_remove) {
			erase_subscriber(t);
		}

		to_remove.clear();
	}

	template<class T>
	void subscribe(T* t, void(T::*f)(Args... args)) {
		auto s = new _Subscriber <T, Args...>(t, f);
		subscribers_.push_back(smart_ptr_type(s));
	}

	void unsubscribe(void* t) final {
		if (firing_) {
			to_remove.push_back(t);
		}
		else {
			erase_subscriber(t);
		}
	}

private:
	void erase_subscriber(void* t) {
		subscribers_.erase(std::remove_if(
			subscribers_.begin(),
			subscribers_.end(),
			[t](auto& s) { return s->instanceof(t); }
		), subscribers_.end());
	}

protected:
	bool firing_ = false;
	std::vector<void*> to_remove;
	std::vector<smart_ptr_type> subscribers_;
};

template<class... Args>
class sorted_event : public event<Args...> {
	struct subscriber_comparer {
		bool operator()(const smart_ptr_type& lhs, const smart_ptr_type& rhs) const {
			return lhs->order() < rhs->order();
		}
	};

public:
	void fire(Args... args) {
		event<Args...>::fire(args...);
		for (auto& s : to_add) {
			add_subscriber(s);
		}

		to_add.clear();
	}

	template <class T>
	void subscribe(T* t, void(T::*f)(Args... args), int order) {
		smart_ptr_type s(new _Subscriber <T, Args...>(t, f, order));
		if (firing_) {
			to_add.push_back(s);
		}
		else {
			add_subscriber(s);
		}
	}

private:
	void add_subscriber(smart_ptr_type& s) {
		subscribers_.insert(std::upper_bound(subscribers_.begin(), subscribers_.end(), s, subscriber_comparer()), s);
	}

private:
	std::vector<smart_ptr_type> to_add;
};
