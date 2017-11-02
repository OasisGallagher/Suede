#pragma once
#include "tools/debug.h"
#include "tools/mathf.h"
#include "internal/memory/memory.h"

template <class T>
class free_list {
	struct Head {
		struct Block* prev;
		struct Block* next;
	};

	struct Block {
		Head head;
		T value;
	};
	
	enum { HeadSize = sizeof(Head) };

public:
	class free_list_iterator {
	public:
		free_list_iterator(Block* block) :ptr_(block) {}

		bool operator == (const free_list_iterator& other) const {
			return ptr_ == other.ptr_;
		}

		bool operator != (const free_list_iterator& other) const {
			return ptr_ != other.ptr_;
		}

		T* operator*() {
			return &ptr_->value;
		}

		free_list_iterator& operator++() {
			ptr_ = ptr_->head.next;
			return *this;
		}

		free_list_iterator operator++(int) {
			free_list_iterator ite = *this;
			ptr_ = ptr_->head.next;
			return ite;
		}

	private:
		Block* ptr_;
	};

	typedef free_list_iterator iterator;

public:
	free_list(size_t capacity) : free_(nullptr), busy_(nullptr), size_(0) {
		capacity_ = capacity;
		allocate(capacity_);
	}

	~free_list() {
		Memory::ReleaseArray(memory_);
	}

	void reallocate(size_t n) {
		Memory::ReleaseArray(memory_);
		size_ = 0;
		capacity_ = n;
		allocate(n);
	}

	iterator begin() { return iterator(busy_); }
	iterator end() { return iterator(nullptr); }

	size_t size() const { return size_; }
	size_t capacity() const { return capacity_; }

	T* pop() {
		AssertX(free_ != nullptr, "out of memory");

		T* result = (T*)advance(free_, HeadSize);
		Block* block = free_;
		free_ = free_->head.next;
		if (free_ != nullptr) {
			free_->head.prev = nullptr;
		}

		block->head.next = busy_;
		block->head.prev = nullptr;

		if (busy_ != nullptr) {
			busy_->head.prev = block;
		}

		busy_ = block;
		++size_;
		return result;
	}

	void push(T* ptr) {
		Block* block = (Block*)advance(ptr, -HeadSize);
		if (busy_ == block) {
			busy_ = busy_->head.next;
		}

		if (block->head.prev != nullptr) {
			block->head.prev->head.next = block->head.next;
		}

		if (block->head.next != nullptr) {
			block->head.next->head.prev = block->head.prev;
		}

		block->head.next = free_;
		block->head.prev = nullptr;

		if (free_ != nullptr) {
			free_->head.prev = block;
		}

		free_ = block;

		--size_;
	}

private:
	void allocate(size_t size) {
		memory_ = Memory::CreateArray<Block>(size);
		for (size_t i = 0; i < size; ++i) {
			if (i >= 1) {
				memory_[i - 1].head.next = memory_ + i;
			}

			if (i < size - 1) {
				memory_[i + 1].head.prev = memory_ + i;
			}
		}

		memory_[size - 1].head.next = nullptr;
		memory_[0].head.prev = nullptr;

		free_ = memory_;
		busy_ = nullptr;
	}

	void* advance(void* ptr, int off) const {
		return (char*)ptr + off;
	}

	size_t size_;
	size_t capacity_;

	Block* free_;
	Block* busy_;
	Block* memory_;
};
