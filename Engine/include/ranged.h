#pragma once
#include <cfloat>
#include "types.h"
#include "tools/math2.h"
#include "debug/debug.h"

template <class T>
class ranged {
public:
	typedef T value_type;

public:
	ranged() : ranged(T()) {}

	explicit ranged(T value, T min = std::numeric_limits<T>::lowest(), T max = std::numeric_limits<T>::max()) {
		reset(value, min, max);
	}

	ranged(const ranged& other) {
		min_ = other.min_; max_ = other.max_; value_ = other.value_;
	}

	ranged& operator= (T value) {
		value_ = Math::Clamp(value, min_, max_);
		return *this;
	}

	void reset(T value, T min, T max) {
		value_ = value;
		setRange(min, max);
	}

	void setValue(T value) {
		value_ = Math::Clamp(value, min_, max_);
	}

	void setRange(T min, T max) {
		if (min > max) {
			Debug::LogError("invalid range");
			max_ = min_ = max;
		}

		min_ = min; max_ = max;
		setValue(value_);
	}

	T min() const { return min_; }
	T max() const { return max_; }
	T value() const { return value_; }

private:
	// do NOT change the order.
	T value_, min_, max_;
};

typedef ranged<int> iranged;
typedef ranged<float> franged;
