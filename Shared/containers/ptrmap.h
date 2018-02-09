#pragma once

#include <map>

template <class Key, class Ty>
class ptr_map {
public:
	typedef Key key_type;
	typedef Ty* value_type;
	typedef std::map<key_type, value_type> container_type;
	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;
	typedef std::pair<iterator, bool> ib_pair;

public:
	~ptr_map() {
		clear();
	}

public:
	bool contains(const key_type& key) {
		return cont_.find(key) != cont_.end();
	}

	bool get(const key_type& key, value_type& value) const {
		const_iterator ite = cont_.find(key);
		if (ite == cont_.end()) {
			return false;
		}

		value = ite->second;
		return true;
	}
	
	void clear() {
		for (iterator ite = cont_.begin(); ite != cont_.end(); ++ite){
			delete ite->second;
		}

		cont_.clear();
	}

	ptr_map<Key, Ty>& operator= (ptr_map<Key, Ty>& other) {
		clear();
		for (iterator ite = other.begin(); ite != other.end(); ++ite) {
			*(operator[](ite->first)) = *ite->second;
		}

		return *this;
	}

	size_t size() const { return cont_.size(); }

	iterator begin() { return cont_.begin(); }
	iterator end() { return cont_.end(); }

	const_iterator cbegin() const { return cont_.cbegin(); }
	const_iterator cend() const { return cont_.cend(); }

	ib_pair insert(const key_type& key) {
		iterator pos = cont_.find(key);
		if (pos != cont_.end()) {
			return std::make_pair(pos, false);
		}

		value_type p = new Ty;
		return cont_.insert(std::make_pair(key, p));
	}
	
	value_type operator[](const key_type& key) {
		iterator pos = cont_.find(key);
		if (pos != cont_.end()){
			return pos->second;
		}

		value_type p = new Ty;
		cont_.insert(std::make_pair(key, p));
		return p;
	}

private:
	container_type cont_;
};
