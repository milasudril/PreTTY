#ifndef PRETTY_BASE_HPP
#define PRETTY_BASE_HPP

#include <tuple>
#include <string_view>
#include <optional>
#include <variant>

namespace pretty
{
	template<class T>
	concept tuple = requires(T x)
	{
		{get<0>(x)};
		{std::tuple_size<T>::value};
	};

	static_assert(tuple<std::pair<int, int>>);

	inline void print(char ch);

	inline void print_raw(std::string_view str);

	inline void print(std::string_view str);

	inline void print(std::string const& str);

	inline void print(char const* c_str);

	template<std::integral T>
	void print(T val);

	template<std::floating_point T>
	void print(T val);

	template<class T>
	void print(std::optional<T> const& x);

	template<class ... T>
	inline void print(std::variant<T...> const& val);

	template<class T>
	requires(tuple<T> && !std::ranges::range<T>)
	void print(T const& x);

	template<class T>
	void print_list_item(T const& val);

	template<class T>
	void print_table_cell(T const& val);

	template<std::ranges::forward_range R>
	void print_table_row(R const& range);

	template<class T>
	requires(tuple<T>)
	void print_table_row(T const& range);

	template<std::ranges::forward_range R>
	void print(R const& range);

	template<std::ranges::forward_range R>
	requires(std::ranges::sized_range<std::ranges::range_value_t<R>>)
	void print(R const& range);

	template<std::ranges::forward_range R>
	requires(tuple<std::ranges::range_value_t<R>> && !std::ranges::range<std::ranges::range_value_t<R>>)
	void print(R const& range);

	template <class F, class Tuple>
	constexpr decltype(auto) apply_adl(F&& f, Tuple&& t);


	#define PRETTY_PRINT_EXPR(expr) \
	pretty::print(std::pair{pretty::code{#expr}, expr})
}

#define PRETTY_BASE_IS_INCLUDED
#include "./base_impl.hpp"

#endif