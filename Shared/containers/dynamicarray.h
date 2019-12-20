// https://github.com/Robbepop/dynarray

#pragma once

#include <memory>
#include <cassert>
#include <initializer_list>

template <typename T>
class dynamic_array {
public:
	using value_type = T;
	using size_type = size_t;
	using reference = value_type &;
	using const_reference = value_type const&;
	using pointer = value_type *;
	using const_pointer = value_type const*;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	dynamic_array() : m_data(nullptr), m_size(0) {}

	template <class... Args>
	dynamic_array(size_type count, Args... args) :
		m_data{ allocate(count, args...) },
		m_size{ count } {
	}

	dynamic_array(size_type count, T const& value) :
		m_data{ allocate(count, value) },
		m_size{ count } {
	}

	dynamic_array(const dynamic_array& other) :
		m_data{ allocate(other) },
		m_size{ other.size() } {
	}

	dynamic_array(dynamic_array&& other) :
		m_data{ allocate(other) },
		m_size{ other.size() } {
	}

	dynamic_array(std::initializer_list<T> list) :
		m_data{ allocate(list) },
		m_size{ list.size() } {
	}

	~dynamic_array() {
		dellocate(m_data);
	}

	template <class... Args>
	void resize(size_type n, Args... args) {
		dellocate(m_data);

		m_size = n;
		m_data = allocate(n, args...);
	}

	void clear() {
		dellocate(m_data);
		m_size = 0;
		m_data = nullptr;
	}

	reference at(size_type pos) {
		assert(pos < size() && "index out of range");
		return *m_data[pos];
	}

	const_reference at(size_type pos) const {
		assert(pos < size() && "index out of range");
		return m_data[pos];
	}

	reference operator[](size_type pos) {
		return m_data[pos];
	}

	const_reference operator[](size_type pos) const {
		return m_data[pos];
	}

	reference front() {
		return m_data[0];
	}

	const_reference front() const {
		return m_data[0];
	}

	reference back() {
		return m_data[size() - 1];
	}

	const_reference back() const {
		return m_data[size() - 1];
	}

	iterator begin() {
		return m_data.get();
	}

	const_iterator begin() const {
		return m_data.get();
	}

	const_iterator cbegin() const {
		return m_data.get();
	}

	iterator end() {
		return m_data.get() + size();
	}

	const_iterator end() const {
		return m_data.get() + size();
	}

	const_iterator cend() const {
		return m_data.get() + size();
	}

	reverse_iterator rbegin() {
		return reverse_iterator{ end() };
	}

	const_reverse_iterator rbegin() const {
		return reverse_iterator{ end() };
	}

	const_reverse_iterator crbegin() const {
		return reverse_iterator{ cend() };
	}

	reverse_iterator rend() {
		return reverse_iterator{ begin() };
	}

	const_reverse_iterator rend() const {
		return reverse_iterator{ begin() };
	}

	const_reverse_iterator crend() const {
		return reverse_iterator{ cbegin() };
	}

	bool empty() const {
		return m_size == 0;
	}

	size_type size() const {
		return m_size;
	}

	dynamic_array& operator=(dynamic_array const& other) = delete;
	dynamic_array& operator=(dynamic_array && other) = delete;
	dynamic_array& operator=(std::initializer_list<T> list) = delete;

private:
	template <class... Args>
	T* allocate(size_t n, Args... args) {
		void* memory = operator new[](n * sizeof(T));
		T* ptr = static_cast<T*>(memory);
		for (int i = 0; i < n; ++i) {
			new(ptr + i) T(args...);
		}

		return (T*)memory;
	}

	T* allocate(dynamic_array&& other) {
		T* memory = allocate(other);
		T* ptr = static_cast<T*>(memory);
		for (int i = 0; i < other.m_size; ++i) {
			new(ptr + i) T(other.m_data[i]);
		}

		other.m_size = 0;
		other.m_data = nullptr;
		return (T**)memory;
	}

	T* allocate(dynamic_array const& other) {
		void* memory = operator new[](other.m_size * sizeof(T));
		T* ptr = static_cast<T*>(memory);
		for (int i = 0; i < other.m_size; ++i) {
			new(ptr + i) T(other.m_data[i]);
		}

		return (T*)memory;
	}

	T* allocate(std::initializer_list<T> list) {
		void* memory = operator new[](list.size() * sizeof(T));
		T* ptr = static_cast<T*>(memory);
		T* value = list.begin();
		for (int i = 0; i < list.size(); ++i) {
			new(ptr + i) T(*value++);
		}

		return (T*)memory;
	}

	void dellocate(T* ptr) {
		for (int i = 0; i < m_size; ++i) {
			ptr[i].~T();
		}

		operator delete[](ptr);
	}

private:
	T* m_data;
	size_type m_size;
};
