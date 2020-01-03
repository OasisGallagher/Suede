// https://stackoverflow.com/a/35856994/2705388

#pragma once

#include <set>
#include <vector>
#include <memory>
#include <algorithm>

/**
 * An event holds a vector of subscribers when it fires, each is called.
 */

template <class... Args>
class _SubscriberBase {
public:
	virtual int order() = 0;
	virtual void* caller() = 0;
	virtual void call(Args... args) = 0;
	virtual bool instanceof(void* t) = 0;
	virtual ~_SubscriberBase() {}
};

template <class T, class... Args>
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
	void* caller() { return t; }
	void call(Args... args) { (t->*f)(args...); }
	bool instanceof(void* _t) { return _t == (void*)t; }
};

/**
 * Our Listener will derive from EventListener<Listener> which holds a list of a events it is subscribed to.
 * As these events will have different sigs, we need a base-class.
 * We will store pointers to this base-class.
 */
class _EventBase {
public:
	virtual ~_EventBase() {}
	virtual void unsubscribe(void* t) = 0;
};

template <class... Args>
class event : public _EventBase {
public:
	event() {}
	event(const event& other) = delete;
	event& operator=(const event&) = delete;

public:
	using smart_ptr_type = std::shared_ptr<_SubscriberBase<Args...>>;
	void raise(Args... args) {
		inside_raise_ = true;
		int size = subscribers_.size();
		for (int i = 0; i < size; ++i) {
			if (to_remove_.find(subscribers_[i]->caller()) == to_remove_.end()) {
				subscribers_[i]->call(args...);
			}
		}

		inside_raise_ = false;

		for (auto& t : to_remove_) {
			erase_subscriber(t);
		}

		to_remove_.clear();
	}

	template <class T>
	void subscribe(T* t, void(T::*f)(Args... args)) {
		subscribers_.emplace_back(new _Subscriber <T, Args...>(t, f));
	}

	void unsubscribe(void* t) {
		if (inside_raise_) {
			to_remove_.insert(t);
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
	bool inside_raise_ = false;
	std::set<void*> to_remove_;
	std::vector<smart_ptr_type> subscribers_;
};

template <class... Args>
class sorted_event : public event<Args...> {
	typedef event<Args...> super;
	struct subscriber_comparer {
		bool operator()(const smart_ptr_type& lhs, const smart_ptr_type& rhs) const {
			return lhs->order() < rhs->order();
		}
	};

public:
	void raise(Args... args) {
		super::raise(args...);
		for (auto& s : to_add_) {
			add_subscriber(s);
		}

		to_add_.clear();
	}

	template <class T>
	void subscribe(T* t, void(T::*f)(Args... args), int order) {
		smart_ptr_type s(new _Subscriber <T, Args...>(t, f, order));
		if (inside_raise_) {
			to_add_.push_back(s);
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
	std::vector<smart_ptr_type> to_add_;
};

#include <mutex>
#include <thread>
#include "unpacker.h"

template <class... Args>
class mt_event : public event<Args...> {
	typedef event<Args...> super;
public:
	mt_event() {
		thread_id_ = std::this_thread::get_id();
	}

public:
	template <class T>
	void subscribe(T* t, void(T::*f)(Args... args)) {
		std::lock_guard<std::recursive_mutex> lock(mutex_);
		super::subscribe(t, f);
	}

	void unsubscribe(void* t) {
		std::lock_guard<std::recursive_mutex> lock(mutex_);
		super::unsubscribe(t);
	}

	void raise(Args... args) {
		if (thread_id_ != std::this_thread::get_id()) {
			delay_raise(args...);
		}
		else {
			std::lock_guard<std::recursive_mutex> lock(mutex_);
			super::raise(args...);
		}
	}

	void delay_raise(Args... args) {
		std::lock_guard<std::recursive_mutex> lock(mutex_);
		raise_arguments_.emplace_back(args...);
	}

	void update() {
		assert(thread_id_ == std::this_thread::get_id());

		std::lock_guard<std::recursive_mutex> lock(mutex_);
		for (auto& argument : raise_arguments_) {
			Unpacker::apply(this, &super::raise, argument);
		}

		raise_arguments_.clear();
	}

private:
	std::thread::id thread_id_;
	std::recursive_mutex mutex_;
	std::vector<std::tuple<Args...>> raise_arguments_;
};
