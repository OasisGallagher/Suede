#pragma once
#include "../debug/debug.h"

template <class T>
class array_list {
public:
	typedef T value_type;
	typedef T& reference;

public:
	array_list(unsigned capacity) : container_(nullptr), free_(-1), head_(-1), tail_(-1) {
		initialize(capacity);
	}

	~array_list() {
		delete[] container_;
	}

private:
	struct node {
		value_type value;
		int index = -1;
		int prev = -1;
		int next = -1;
		int free = -1;
	};

public:
	class array_list_iterator {
	public:
		array_list_iterator(array_list<value_type>* list, int current) {
			list_ = list;
			current_ = current;
		}

		int index() const {
			return current_;
		}

		array_list_iterator* operator ->() {
			return this;
		}

		value_type& operator *() {
			return (*list_)[current_];
		}

		bool operator == (const array_list_iterator& other) const {
			return current_ == other.current_;
		}

		bool operator != (const array_list_iterator& other) const {
			return current_ != other.current_;
		}

		array_list_iterator& operator++() {
			current_ = list_->container_[current_].next;
			return *this;
		}

		array_list_iterator operator++(int) {
			free_list_iterator ite = *this;
			current_ = list_->container_[current_].next;
			return ite;
		}

	private:
		int current_;
		array_list<value_type>* list_;
	};

	typedef array_list_iterator iterator;

public:
	int add(const value_type& value) {
		int pos = pop_free();
		if (pos < 0) {
			return -1;
		}

		if (tail_ == -1) {
			head_ = pos;
			container_[pos].prev = -1;
		}
		else {
			container_[tail_].next = pos;
			container_[pos].prev = tail_;
		}

		container_[pos].value = value;
		tail_ = pos;
		++size_;

		return pos;
	}

	int erase(int index) {
		int next = container_[index].next;

		node& n = container_[index];
		if (n.prev != -1) { container_[n.prev].next = n.next; }
		if (n.next != -1) { container_[n.next].prev = n.prev; }
		if (n.index == head_) { head_ = n.next; }
		if (n.index == tail_) { tail_ = n.prev; }

		push_list(n);
		if (size_ == 0) { Debug::Break(); }
		--size_;
		return next;
	}

	int next_index(int current) {
		int answer = container_[current].next;
		if (answer < 0) { answer = head_; }
		return answer;
	}

	int prev_index(int current) {
		int answer = container_[current].prev;
		if (answer < 0) { answer = tail_; }
		return answer;
	}

	unsigned size() const { return size_; }
	reference operator[](unsigned index) { return container_[index].value; }

	reference next_value(int current) { return container_[next_index(current)].value; }
	reference prev_value(int current) { return container_[prev_index(current)].value; }

	int front_index() const { return head_; }
	int back_index() const { return tail_; }

	iterator end() { return iterator(this, -1); }
	iterator begin() { return iterator(this, head_); }

private:
	array_list& operator = (const array_list& other);

	void initialize(unsigned capacity) {
		capacity_ = capacity;
		container_ = new node[capacity];

		for (int i = 0; i < capacity_; ++i) {
			container_[i].index = i;

			if (i < capacity_ - 1) {
				container_[i].free = i + 1;
			}
		}

		size_ = 0;
		free_ = 0;
	}

	int pop_free() {
		if (free_ == -1) {
			Debug::LogError("out of memory");
			return -1;
		}

		int answer = free_;
		free_ = container_[free_].free;
		container_[answer].free = -1;

		return answer;
	}

	void push_list(node& n) {
		n.value = T();
		n.prev = n.next = -1;
		n.free = free_;
		free_ = n.index;
	}

	unsigned size_;
	unsigned capacity_;

	int free_;
	int head_;
	int tail_;
	node* container_;
};
