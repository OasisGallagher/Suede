#include <vector>

class GUIPrivate {
	friend class GUI;

	template <class T>
	static inline std::string ToString(const T & item) {
		return std::to_string(item);
	}

	template <>
	static inline std::string ToString(const std::string & item) {
		return item;
	}

	static void CopyChars(std::vector<char>& buffer, int& bi, const std::string& title) {
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
	static const char* Join(T first, T last) {
		static std::vector<char> buffer(16);

		int bi = 0;
		for (; first != last; ++first) {
			CopyChars(buffer, bi, ToString(*first));
		}

		buffer[bi++] = 0;
		buffer[bi] = 0;

		return buffer.data();
	}

	template<class T>
	static const char* JoinEnums(T value, int* selected) {
		static std::vector<char> buffer(16);

		int pos = -1, bi = 0;
		for (int i = 0; i < T::size(); ++i) {
			if (value == T::value(i)) { pos = i; }
			CopyChars(buffer, bi, T::value(i).to_string());
		}

		buffer[bi++] = 0;
		buffer[bi] = 0;

		if (selected != nullptr) { *selected = pos; }

		return buffer.data();
	}

	template<class T>
	static const char* JoinEnumMasks(T value) {
		static std::vector<char> buffer(16);

		int bi = 0;

		// skip None & Everything.
		for (int i = 2; i < T::size(); ++i) {
			CopyChars(buffer, bi, T::value(i).to_string());
		}

		buffer[bi++] = 0;
		buffer[bi] = 0;

		return buffer.data();
	}
};

template <class T>
inline bool GUI::EnumPopup(const char* title, T value, int& selected) {
	const char* items = GUIPrivate::JoinEnums(value, &selected);
	return Popup(title, &selected, items);
}

template <class T>
inline bool GUI::EnumMaskPopup(const char* title, T value) {
	const char* items = GUIPrivate::JoinEnumMasks(value);
	return MaskPopup(title, value, items);
}

template <class T>
inline bool GUI::Popup(const char* title, int* selected, T first, T last) {
	return Popup(title, selected, GUIPrivate::Join(first, last));
}
