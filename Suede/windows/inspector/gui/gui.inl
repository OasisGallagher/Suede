class GUIPrivate {
	friend class GUI;

	template<class T>
	static inline std::string toString(const T & item) {
		return std::to_string(item);
	}

	template<>
	static inline std::string toString(const std::string & item) {
		return item;
	}

	static void copyChars(std::vector<char>& buffer, int& bi, const std::string& title) {
		while (bi + title.length() + int(bi != 0) + 2 > buffer.size()) {
			buffer.resize(buffer.size() * 2);
		}

		if (bi != 0) {
			buffer[bi++] = 0;
		}

		memcpy(&buffer[bi], title.c_str(), title.length());
		bi += title.length();
	}

	template<class T>
	static const char* join(T first, T last) {
		static std::vector<char> buffer(16);

		int bi = 0;
		for (; first != last; ++first) {
			copyChars(buffer, bi, toString(*first));
		}

		buffer[bi++] = 0;
		buffer[bi] = 0;

		return buffer.data();
	}

	template<class T>
	static const char* joinEnums(T value, int* selected) {
		static std::vector<char> buffer(16);

		int pos = -1, bi = 0;
		for (int i = 0; i < T::size(); ++i) {
			if (value == T::value(i)) { pos = i; }
			copyChars(buffer, bi, T::value(i).to_string());
		}

		buffer[bi++] = 0;
		buffer[bi] = 0;

		if (selected != nullptr) { *selected = pos; }

		return buffer.data();
	}

	static bool comboImpl(const char* title, int* selected, const char* items);
};

template <class T>
inline bool GUI::enums(const char* title, T value, int& selected) {
	const char* items = GUIPrivate::joinEnums(value, &selected);
	return GUIPrivate::comboImpl(title, &selected, items);
}

template <class T>
inline bool GUI::combo(const char* title, int* selected, T first, T last) {
	return GUIPrivate::comboImpl(title, selected, GUIPrivate::join(first, last));
}
