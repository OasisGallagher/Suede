#pragma once

#include "containers/table.h"
#include "grammar/grammarsymbol.h"

class GrammarContainer;

class Forwards {
public:
	Forwards() {}

public:
	typedef std::set<GrammarSymbolPtr> container_type;
	typedef container_type::iterator iterator;
	typedef container_type::const_iterator const_iterator;

public:
	bool operator < (const Forwards& other) const;

public:
	int size() const { return cont_.size(); }
	iterator begin() { return cont_.begin(); }
	iterator end() { return cont_.end(); }

	const_iterator begin() const { return cont_.begin(); }
	const_iterator end() const { return cont_.end(); }

	void erase(const GrammarSymbolPtr& symbol);
	bool insert(const GrammarSymbolPtr& symbol);

private:
	container_type cont_;
};

class LR1Item : public intrusive_ref_counter {
public:
	LR1Item() : cpos_(0), dpos_(0) {}
	LR1Item(int cpos, int dpos) : cpos_(cpos), dpos_(dpos) {}
	LR1Item(int cpos, int dpos, const Forwards& forwards) : cpos_(cpos), dpos_(dpos), forwards_(forwards) {}

	int Compare(const LR1Item& other) const;

	int GetCpos() const { return cpos_; }
	void SetCpos(int pos) { cpos_ = pos; }

	int GetDpos() const { return dpos_; }
	int SetDpos(int pos) { return dpos_ = pos; }

	Forwards& GetForwards() { return forwards_; }
	const Forwards& GetForwards() const { return forwards_; }

	bool IsCore() const { return dpos_ != 0 || cpos_ == 0; }

	std::string ToRawString() const;
	std::string ToString(const GrammarContainer& grammars) const;

private:
	int cpos_, dpos_;
	Forwards forwards_;
};

typedef intrusive_ptr<LR1Item> LR1ItemPtr;

struct LR1ItemComparer {
	bool operator () (const LR1ItemPtr& lhs, const LR1ItemPtr& rhs)const {
		return lhs->Compare(*rhs) < 0;
	}
};

class LR1Itemset : public intrusive_ref_counter {
public:
	LR1Itemset() {}

public:
	typedef std::set<LR1ItemPtr, LR1ItemComparer> container_type;

	typedef container_type::iterator iterator;
	typedef container_type::const_iterator const_iterator;

public:
	iterator begin() { return container_.begin(); }
	const_iterator begin() const { return container_.begin(); }

	iterator end() { return container_.end(); }
	const_iterator end() const { return container_.end(); }

	iterator find(const LR1ItemPtr& item) { return container_.find(item); }
	const_iterator find(const LR1ItemPtr& item) const { return container_.find(item); }

	void clear() { container_.clear(); }
	int size() const { return container_.size(); }
	bool empty() const { return container_.empty(); }

	bool insert(const LR1ItemPtr& item);

public:
	int Compare(const LR1Itemset& other) const;

public:
	const std::string& GetName() const;
	void SetName(const std::string& name);

	std::string ToString(const GrammarContainer& grammars) const;

private:
	std::string name_;
	container_type container_;
};

typedef intrusive_ptr<LR1Itemset> LR1ItemsetPtr;

struct LR1ItemsetComparer {
	bool operator () (const LR1ItemsetPtr& lhs, const LR1ItemsetPtr& rhs)const {
		return lhs->Compare(*rhs) < 0;
	}
};

class LR1ItemsetContainer : public std::set <LR1ItemsetPtr, LR1ItemsetComparer> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class Propagations : public std::map <LR1ItemPtr, LR1ItemsetPtr, LR1ItemComparer> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

struct LR1EdgeComparer {
	bool operator() (const std::pair<LR1ItemsetPtr, GrammarSymbolPtr>& lhs, const std::pair<LR1ItemsetPtr, GrammarSymbolPtr>& rhs) const {
		int c = lhs.first->Compare(*rhs.first);
		if (c != 0) {
			return c < 0;
		}

		return lhs.second < rhs.second;
	}
};

class LR1EdgeTable : public table <LR1ItemsetPtr, GrammarSymbolPtr, LR1ItemsetPtr, LR1EdgeComparer> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};
