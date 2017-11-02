#pragma once

#include <map>

template <class Ty>
class PtrMap {
public:
	typedef Ty* value_type;
	typedef std::map<std::string, value_type> container_type;
	typedef typename container_type::iterator iterator;

public:
	~PtrMap() {
		clear();
	}

public:
	bool contains(const std::string& name) {
		return cont_.find(name) != cont_.end();
	}

	bool get(const std::string& name, value_type& value) {
		iterator ite = cont_.find(name);
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

	iterator begin() { return cont_.begin(); }
	iterator end() { return cont_.end(); }

	value_type operator[](const std::string& name) {
		iterator pos = cont_.find(name);
		if (pos != cont_.end()){
			return pos->second;
		}

		value_type p = new Ty;
		cont_.insert(std::make_pair(name, p));
		return p;
	}

private:
	container_type cont_;
};