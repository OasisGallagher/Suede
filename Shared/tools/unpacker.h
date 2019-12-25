#pragma once
#include <tuple>

// https://stackoverflow.com/a/12650100/2705388
class Unpacker {
	template<size_t N>
	struct ApplyMember {
		template<typename C, typename F, typename T, typename... A>
		static inline auto apply(C&& c, F&& f, T&& t, A&&... a) ->
			decltype(ApplyMember<N - 1>::apply(std::forward<C>(c), std::forward<F>(f), std::forward<T>(t), std::get<N - 1>(std::forward<T>(t)), std::forward<A>(a)...)) {
			return ApplyMember<N - 1>::apply(std::forward<C>(c), std::forward<F>(f), std::forward<T>(t), std::get<N - 1>(std::forward<T>(t)), std::forward<A>(a)...);
		}
	};

	template<>
	struct ApplyMember<0> {
		template<typename C, typename F, typename T, typename... A>
		static inline auto apply(C&& c, F&& f, T&&, A&&... a) ->
			decltype((std::forward<C>(c)->*std::forward<F>(f))(std::forward<A>(a)...)) {
			return (std::forward<C>(c)->*std::forward<F>(f))(std::forward<A>(a)...);
		}
	};

public:
	// C is the class, F is the member function, T is the tuple.
	template<typename C, typename F, typename T>
	static inline auto apply(C&& c, F&& f, T&& t) ->
		decltype(ApplyMember<std::tuple_size<typename std::decay<T>::type>::value>::apply(std::forward<C>(c), std::forward<F>(f), std::forward<T>(t))) {
		return ApplyMember<std::tuple_size<typename std::decay<T>::type>::value>::apply(std::forward<C>(c), std::forward<F>(f), std::forward<T>(t));
	}
};
