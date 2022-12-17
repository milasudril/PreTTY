#ifndef PRETTY_BASE_IS_INCLUDED
#error Please include base.hpp instead
#endif

#include <array>
#include <charconv>
#include <cstdio>
#include <algorithm>
#include <limits>


void pretty::print(char ch)
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

void pretty::print_raw(std::string_view str)
{
	std::ranges::for_each(str, [](auto item){putchar(item);});
}

void pretty::print(std::string_view str)
{
	std::ranges::for_each(str, [](auto x) { print(x); });
}

void pretty::print(std::string const& str)
{
	print(std::string_view{str});
}

void pretty::print(char const* c_str)
{
	print(std::string_view{c_str});
}

template<std::integral T>
void pretty::print(T val)
{
	static constexpr auto num_chars = std::numeric_limits<T>::digits10 + 3;
	std::array<char, num_chars> buffer{};
	std::to_chars(std::data(buffer), std::data(buffer) + std::size(buffer) - 1, val);
	print(std::data(buffer));
}

template<std::floating_point T>
void pretty::print(T val)
{
	static constexpr auto num_chars = 32;
	std::array<char, num_chars> buffer{};
	std::to_chars(std::data(buffer), std::data(buffer) + std::size(buffer) - 1, val);
	print(std::data(buffer));
}

template<class T>
void pretty::print(std::optional<T> const& x)
{
	if(x.has_value())
	{ print(*x); }
	else
	{ puts("<span class=\"empty\">(no value)</span>"); }
}

template<class ... T>
void pretty::print(std::variant<T...> const& val)
{
	std::visit([](auto const& item){ print(item);}, val);
}

template<class T>
requires(pretty::tuple<T> && !std::ranges::range<T>)
void pretty::print(T const& x)
{
	puts("<ol start=\"0\" class=\"range_content\">");
	apply_adl([](auto const&... args){
		(print_list_item(args),...);
	}, x);
	puts("</ol>");
}

template<class T>
void pretty::print_list_item(T const& val)
{
	print_raw("<li>");
	print(val);
	print_raw("</li>");
}

template<class T>
void pretty::print_table_cell(T const& val)
{
	print_raw("<td>");
	print(val);
	print_raw("</td>");
}

template<std::ranges::forward_range R>
void pretty::print_table_row(R const& range)
{
	puts("<tr>");
	std::ranges::for_each(range, [](auto const& item) {
		print_table_cell(item);
	});
	puts("</tr>");
}

template<pretty::tuple T>
void pretty::print_table_row(T const& item)
{
	puts("<tr>");
	apply_adl([](auto const&... args){
		(print_table_cell(args),...);
	}, item);
	puts("</tr>");
}

template<std::ranges::forward_range R>
void pretty::print(R const& range)
{
	puts("<ol start=\"0\" class=\"range_content\">");
	std::ranges::for_each(range, [](auto const& item){ print_list_item(item); });
	puts("</ol>");
}

template<pretty::fwd_range_of_sized_range R>
void pretty::print(R const& range)
{
	if constexpr(fwd_range_of_constexpr_sized_range<R>)
	{
		puts("<table>");
		std::ranges::for_each(range, [](auto const& range){
			print_table_row(range);
		});
		puts("</table>");
	}
	else
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
}

template<class R>
requires(pretty::fwd_range_of_tuple<R> && !pretty::fwd_range_of_sized_range<R>)
void pretty::print(R const& range)
{
	puts("<table>");
	std::ranges::for_each(range, [](auto const& item){
		print_table_row(item);
	});
	puts("</table>");
}

namespace pretty::detail
{
	template <class F, class Tuple, std::size_t... I>
	constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
	{
		return std::invoke(std::forward<F>(f), get<I>(std::forward<Tuple>(t))...);
	}
}

template <class F, class Tuple>
constexpr decltype(auto) pretty::apply_adl(F&& f, Tuple&& t)
{
	return detail::apply_impl(
		std::forward<F>(f), std::forward<Tuple>(t),
		std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
}

template<class T>
void pretty::print_labeled_value(std::string_view label, T const& value)
{
	puts("<table class=\"single_row\">");
	print_table_row(std::tuple{label, "=", value});
	puts("</table>");
}

template<class T>
void pretty::print_labeled_value(char const* label, T const& value)
{
	puts("<table class=\"single_row\">");
	print_table_row(std::tuple{std::string_view{label}, "=", value});
	puts("</table>");
}