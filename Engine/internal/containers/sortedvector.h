#pragma once
#include <vector>
#include <algorithm>

template <class Ty, class Comp = std::less<Ty> > 
class sorted_vector {
	typedef std::vector<Ty> container_type;

public:
	typedef Ty value_type;
	typedef Comp comparer_type;

	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;
	typedef typename container_type::reference reference;
	typedef typename container_type::const_reference const_reference;

public:
	void insert(const value_type& value) {
		iterator ite = find(value);
		if (ite != container_.end() && !comp_(*ite, value)) {
			*ite = value;
		}
		else {
			container_.insert(ite, value);
		}
	}

	iterator find(const value_type& value) {
		return std::lower_bound(container_.begin(), container_.end(), value, comp_);
	}

	bool get(value_type& value) {
		iterator ite = find(value);
		if (ite == container_.end()) { return false; }
		value = *ite;
		return true;
	}

	void remove(const value_type& value) {
		iterator ite = find(value);
		if (!comp_(value, *ite)) {
			container_.erase(ite);
		}
	}

	bool empty() { return container_.empty(); }

	void contains(const value_type& value) {
		iterator ite = find(value);
		return ite != container_.end() && !comp(value, *ite);
	}

	iterator begin() { return container_.begin(); }
	iterator end() { return container_.end(); }

	size_t size() const { return container_.size(); }

	reference back() { return container_.back(); }
	reference front() { return container_.front(); }

	reference at(size_t i) { return container_.at(i); }
	reference operator[] (size_t i) { return container_[i]; }

private:
	comparer_type comp_;
	container_type container_;
};
