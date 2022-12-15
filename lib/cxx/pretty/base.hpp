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
		{std::get<0>(x)};
		{std::tuple_size<T>::value};
	};

	template<class First, class Second>
	void print(std::pair<First, Second> const& val);

	template<class T>
	void print(std::optional<T> const& x);

	template<class T>
	requires(is_tuple<T>)
	void print(T const& x);

	template<class ... T>
	inline void print(std::variant<T...> const& val);

	template<std::ranges::input_range R>
	requires(!is_pair<std::ranges::range_value_t<R>>)
	void print(R&& range);

	template<std::ranges::input_range R>
	requires(is_pair<std::ranges::range_value_t<R>>)
	void print(R&& range);



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

	template<std::ranges::input_range R>
	requires(!is_pair<std::ranges::range_value_t<R>>)
	void print(R&& range)
	{
		puts("<ol start=\"0\" class=\"range_content\">");
		std::ranges::for_each(range, [](auto const& item){ print_list_item(item); });
		puts("</ol>");
	}

	template<std::ranges::input_range R>
	requires(is_pair<std::ranges::range_value_t<R>>)
	void print(R&& range)
	{
		puts("<table><tr><th>First</th><th>Second</th></tr>");
		std::ranges::for_each(range, [](auto const& item){
			puts("<tr><td>");
			print(item.first);
			puts("</td><td>");
			print(item.second);
			puts("</td></tr>");
		});
		puts("</table>");
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

	template<class T>
	requires(is_tuple<T>)
	void print(T const& x)
	{
		puts("<ol start=\"0\" class=\"range_content\">");
		std::apply([](auto const&... args){
			(print_list_item(args),...);
		}, x);
		puts("</ol>");
	}

#define PRETTY_PRINT_EXPR(expr) \
pretty::print(std::pair{pretty::code{#expr}, expr})
}

#endif