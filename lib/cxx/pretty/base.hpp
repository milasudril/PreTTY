#ifndef PRETTY_BASE_HPP
#define PRETTY_BASE_HPP

#include <string_view>
#include <cstdio>
#include <algorithm>
#include <string>
#include <variant>
#include <limits>
#include <charconv>
#include <optional>

namespace pretty
{
	inline void print(char ch);

	inline void print_raw(std::string_view str);

	inline void print(std::string_view str);

	inline void print(std::string const& str);

	inline void print(char const* c_str);

	template<class T>
	void print_list_item(T const& val);

	template<std::integral T>
	void print(T val);

	template<std::floating_point T>
	void print(T val);

	template<class T>
	concept is_pair = requires(T x)
	{
		{x.first};
		{x.second};
	};

	template<class T>
	concept is_tuple = !is_pair<T> && requires(T x)
	{
		{get<0>(x)};
		{std::tuple_size<T>::value};
	};

	template<class First, class Second>
	void print(std::pair<First, Second> const& val);

	template<class T>
	void print(std::optional<T> const& x);

	template<class T>
	requires(is_tuple<T> && !std::ranges::range<T>)
	void print(T const& x);

	template<class ... T>
	inline void print(std::variant<T...> const& val);

	template<std::ranges::forward_range R>
	void print(R const& range);

	template<std::ranges::forward_range R>
	requires(is_pair<std::ranges::range_value_t<R>>)
	void print(R const& range);

	template<std::ranges::forward_range R>
	requires(std::ranges::sized_range<std::ranges::range_value_t<R>>)
	void print(R const& range);



	/// implementation

	template<std::integral T>
	void print(T val)
	{
		static constexpr auto num_chars = std::numeric_limits<T>::digits10 + 3;
		std::array<char, num_chars> buffer{};
		std::to_chars(std::data(buffer), std::data(buffer) + std::size(buffer) - 1, val);
		print(std::data(buffer));
	}

	template<std::floating_point T>
	void print(T val)
	{
		static constexpr auto num_chars = std::numeric_limits<T>::digits10 + 5;
		std::array<char, num_chars> buffer{};
		std::to_chars(std::data(buffer), std::data(buffer) + std::size(buffer) - 1, val);
		print(std::data(buffer));
	}

	void print(std::string_view str)
	{
		std::ranges::for_each(str, [](auto x) {
			print(x);
		});
	}

	void print(std::string const& str)
	{
		print(std::string_view{str});
	}

	void print(char const* c_str)
	{
		print(std::string_view{c_str});
	}

	template<class ... T>
	inline void print(std::variant<T...> const& val)
	{
		std::visit([](auto const& item){ print(item);}, val);
	}

	template<class First, class Second>
	void print(std::pair<First, Second> const& val)
	{
		puts("<table class=\"pair\">");
		puts("<tr><td>");print(val.first);puts("</td><td>");print(val.second);puts("</td></tr>");
		puts("</table>");
	}

	template<class T>
	void print_list_item(T const& val)
	{
		print_raw("<li>");
		print(val);
		print_raw("</li>");
	}

	template<std::ranges::forward_range R>
	void print(R const& range)
	{
		puts("<ol start=\"0\" class=\"range_content\">");
		std::ranges::for_each(range, [](auto const& item){ print_list_item(item); });
		puts("</ol>");
	}

	template<std::ranges::forward_range R>
	requires(is_pair<std::ranges::range_value_t<R>>)
	void print(R const& range)
	{
		puts("<table>");
		std::ranges::for_each(range, [](auto const& item){
			puts("<tr><td>");
			print(item.first);
			puts("</td><td>");
			print(item.second);
			puts("</td></tr>");
		});
		puts("</table>");
	}

	template<std::ranges::forward_range R>
	void print_table_row(R const& range)
	{
		puts("<tr>");
		std::ranges::for_each(range, [](auto const& item) {
			puts("<td>");
			print(item);
			puts("</td>");
		});
		puts("</tr>");
	}

	template<std::ranges::forward_range R>
	requires(std::ranges::sized_range<std::ranges::range_value_t<R>>)
	void print(R const& range)
	{
		auto const i = std::ranges::adjacent_find(range, [](auto const& a, auto const& b) {
			return std::size(a) != std::size(b);
		});

		if(i == std::end(range))
		{
			puts("<table>");
			std::ranges::for_each(range, [](auto const& range){
				print_table_row(range);
			});
			puts("</table>");
		}
		else
		{
			puts("<ol start=\"0\">");
			std::ranges::for_each(range, [](auto const& range) {
				print_list_item(range);
			});
			puts("</ol>");
		}
	}

	inline void print(char ch)
	{
		switch(ch)
		{
		case '&':
			printf("&amp;");
			break;
		case '<':
			printf("&lt;");
			break;
		case '>':
			printf("&gt;");
			break;
		case '"':
			printf("&quot;");
			break;

		default:
			putchar(ch);
		}
	}

	inline void print_raw(std::string_view str)
	{
		std::ranges::for_each(str, [](auto item){putchar(item);});
	}

	template<class T>
	void print(std::optional<T> const& x)
	{
		if(x.has_value())
		{ print(*x); }
		else
		{ puts("<span class=\"empty\">(no value)</span>"); }
	}

	namespace detail
	{
		template <class F, class Tuple, std::size_t... I>
		constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
		{
			return std::invoke(std::forward<F>(f), get<I>(std::forward<Tuple>(t))...);
		}
	}

	template <class F, class Tuple>
	constexpr decltype(auto) apply_adl(F&& f, Tuple&& t)
	{
		return detail::apply_impl(
			std::forward<F>(f), std::forward<Tuple>(t),
			std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
	}

	template<class T>
	requires(is_tuple<T> && !std::ranges::range<T>)
	void print(T const& x)
	{
		puts("<ol start=\"0\" class=\"range_content\">");
		apply_adl([](auto const&... args){
			(print_list_item(args),...);
		}, x);
		puts("</ol>");
	}

#define PRETTY_PRINT_EXPR(expr) \
pretty::print(std::pair{pretty::code{#expr}, expr})
}

#endif