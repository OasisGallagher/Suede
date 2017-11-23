#pragma once

#include <map>

template <class Key, class Ty>
class PtrMap {
public:
	typedef Key key_type;
	typedef Ty* value_type;
	typedef std::map<key_type, value_type> container_type;
	typedef typename container_type::iterator iterator;

public:
	~PtrMap() {
		clear();
	}

public:
	bool contains(const key_type& key) {
		return cont_.find(key) != cont_.end();
	}

	bool get(const key_type& key, value_type& value) {
		iterator ite = cont_.find(key);
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

	size_t size() const { return cont_.size(); }

	iterator begin() { return cont_.begin(); }
	iterator end() { return cont_.end(); }

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