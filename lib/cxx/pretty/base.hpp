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

	template<class T>
	concept fwd_range_of_sized_range = std::ranges::forward_range<T>
		&& std::ranges::sized_range<std::ranges::range_value_t<T>>;

	template<class T>
	concept fwd_range_of_tuple = std::ranges::forward_range<T>
		&& tuple<std::ranges::range_value_t<T>>;

	template<class T>
	concept constexpr_sized_range = std::ranges::sized_range<T> &&
		requires(T x)
	{
		{ std::bool_constant<(std::size(T{}), true)> {} } -> std::same_as<std::true_type>;
	};

	template<class T>
	concept fwd_range_of_constexpr_sized_range = std::ranges::forward_range<T>
		&& constexpr_sized_range<std::ranges::range_value_t<T>>;

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

	template<tuple T>
	void print_table_row(T const& range);

	template<std::ranges::forward_range R>
	void print(R const& range);

	template<fwd_range_of_sized_range R>
	void print(R const& range);

	template<class R>
	requires(fwd_range_of_tuple<R> && !fwd_range_of_sized_range<R>)
	void print(R const& range);

	template<class F, class Tuple>
	constexpr decltype(auto) apply_adl(F&& f, Tuple&& t);

	template<class T>
	void print_labeled_value(char const* label, T const& value);

	template<class T>
	void print_labeled_value(std::string_view label, T const& value);

	#define PRETTY_PRINT_EXPR(expr) \
	pretty::print_labeled_value(#expr, expr)
}

#define PRETTY_BASE_IS_INCLUDED
#include "./base_impl.hpp"

#endif