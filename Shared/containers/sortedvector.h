#pragma once
#include <vector>
#include <algorithm>

template <class Ty, class Comp = std::less<Ty> > 
class sorted_vector {
public:
	typedef Ty value_type;
	typedef Comp comparer_type;
	typedef std::vector<value_type> container_type;

	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;
	typedef typename container_type::reference reference;
	typedef typename container_type::const_reference const_reference;

public:
	sorted_vector() {}

	template <class Iterator>
	sorted_vector(Iterator first, Iterator last) {
		for (; first != last; ++first) {
			insert(*first);
		}
	}

	void insert(const value_type& value) {
		container_.insert(find(value), value);
	}

	const container_type& container() const {
		return container_;
	}

	void sort() {
		std::sort(container_.begin(), container_.end(), comp_);
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

	void erase(const value_type& value) {
		iterator ite = find(value);
		if (ite != end() && !comp_(value, *ite)) {
			container_.erase(ite);
		}
	}

	bool empty() { return container_.empty(); }

	bool contains(const value_type& value) {
		iterator ite = find(value);
		return ite != container_.end() && !comp_(value, *ite);
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
