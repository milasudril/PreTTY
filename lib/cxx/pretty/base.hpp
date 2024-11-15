#ifndef PRETTY_BASE_HPP
#define PRETTY_BASE_HPP

#include <tuple>
#include <string_view>
#include <optional>
#include <variant>
#include <mutex>
#include <charconv>
#include <array>

namespace pretty
{
	template<class T>
	concept tuple = requires(T x)
	{
		{get<0>(x)};
		{std::tuple_size<T>::value};
	};

	template<tuple T, size_t Index>
	using tuple_element_t = std::decay_t<decltype(get<Index>(std::declval<T>()))>;

	template<class T>
	concept has_size = requires(T x)
	{
		{std::size(x)};
	};

	template<std::ranges::forward_range R>
	void print_table_row(R const& range);

	template<tuple T>
	requires(!std::ranges::forward_range<T>)
	void print_table_row(T const& range);

	void print_table_row(std::string) = delete;
	void print_table_row(std::string_view) = delete;

	template<class T>
	concept has_table_row_formatter = requires (T x)
	{
		{ print_table_row(x) };
	};

	static_assert(!has_table_row_formatter<std::string>);
	static_assert(!has_table_row_formatter<std::string_view>);

	template<tuple T, size_t Index>
	constexpr bool elements_have_table_row_formatter();

	template<class T>
	constexpr std::optional<size_t> generic_size(T const& obj);

	template<class F, class Tuple>
	constexpr decltype(auto) apply_adl(F&& f, Tuple&& t);

	template<size_t Index , tuple T>
	constexpr bool elements_have_same_size(T const& t);

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

	inline void write_as_html(char ch);

	inline void write_as_html(std::byte val);

	inline void write_raw(std::string_view str);

	inline void write_as_html(std::string_view str);

	inline void write_as_html(std::string const& str);

	inline void write_as_html(char const* c_str);

	template<std::integral T>
	constexpr auto to_char_buffer(T val)
	{
		constexpr auto num_chars = std::numeric_limits<T>::digits10 + 3;
		std::array<char, num_chars> buffer{};
		std::to_chars(std::data(buffer), std::data(buffer) + std::size(buffer) - 1, val);
		return buffer;
	}

	template<std::floating_point T>
	constexpr auto to_char_buffer(T val)
	{
		constexpr auto num_chars = 32;
		std::array<char, num_chars> buffer{};
		std::to_chars(std::data(buffer), std::data(buffer) + std::size(buffer) - 1, val);
		return buffer;
	}

	template<std::integral T>
	void write_as_html(T val);

	template<std::floating_point T>
	void write_as_html(T val);

	template<class T>
	void write_as_html(std::optional<T> const& x);

	template<class T>
	void write_as_html(T const* ptr);

	template<class ... T>
	inline void write_as_html(std::variant<T...> const& val);

	template<class T>
	requires(tuple<T> && !std::ranges::range<T>)
	void write_as_html(T const& x);

	template<class T>
	void print_list_item(T const& val);

	template<class T>
	void print_table_cell(T const& val);

	template<std::ranges::forward_range R>
	void write_as_html(R const& range);

	template<fwd_range_of_sized_range R>
	void write_as_html(R const& range);

	template<class R>
	requires(fwd_range_of_tuple<R> && !fwd_range_of_sized_range<R>)
	void write_as_html(R const& range);

	inline constinit std::recursive_mutex output_mutex;

	template<class Function, class ... Args>
	void atomic_write(Function&& func, Args&&... args);

	template<class T>
	void print(T const& val);

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
