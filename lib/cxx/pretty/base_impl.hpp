#ifndef PRETTY_BASE_IS_INCLUDED
#error Please include base.hpp instead
#endif

#include <cstdio>
#include <algorithm>
#include <limits>
#include <functional>

#include <typeinfo>


template<class T>
constexpr std::optional<size_t> pretty::generic_size(T const& obj)
{
	if constexpr(tuple<T>)
	{
		return std::tuple_size_v<T>;
	}
	else
	if constexpr(has_size<T>)
	{
		return std::size(obj);
	}
	else
	{
		return std::nullopt;
	}
}


template<pretty::tuple T, size_t Index = 0>
constexpr bool pretty::elements_have_table_row_formatter()
{
	if constexpr(Index == std::tuple_size_v<T>)
	{
		return true;
	}
	else
	{
		using current_element = tuple_element_t<T, Index>;
		return has_table_row_formatter<current_element>
			&& elements_have_table_row_formatter<T, Index + 1>();
	}
}

template<size_t Index = 1, pretty::tuple T>
constexpr bool pretty::elements_have_same_size(T const& t)
{
	static_assert(Index != 0);
	if constexpr(Index != std::tuple_size_v<T>)
	{
		auto const size_prev = generic_size(get<Index - 1>(t));
		auto const size_current = generic_size(get<Index>(t));

		if(size_prev.has_value() && size_current.has_value())
		{
			return *size_prev == size_current && elements_have_same_size<Index + 1>(t);
		}
		return false;
	}
	else
	{
		return true;
	}
}

void pretty::write_as_html(char ch)
{
	switch(ch)
	{
	case '&':
		printf("&");
		break;
	case '<':
		printf("<");
		break;
	case '>':
		printf(">");
		break;
	case '"':
		printf("&quot;");
		break;

	default:
		putchar(ch);
	}
}

void pretty::write_as_html(std::byte val)
{
	printf("<code class=\"byte\">%02x</code>", static_cast<uint8_t>(val));
}

void pretty::write_raw(std::string_view str)
{
	std::ranges::for_each(str, [](auto item){putchar(item);});
}

void pretty::write_as_html(std::string_view str)
{
	std::ranges::for_each(str, [](auto x) { write_as_html(x); });
}

void pretty::write_as_html(std::string const& str)
{
	write_as_html(std::string_view{str});
}

void pretty::write_as_html(char const* c_str)
{
	write_as_html(std::string_view{c_str});
}

template<std::integral T>
void pretty::write_as_html(T val)
{
	write_as_html(std::data(to_char_buffer(val)));
}

template<std::floating_point T>
void pretty::write_as_html(T val)
{
	write_as_html(std::data(to_char_buffer(val)));
}

template<class T>
void pretty::write_as_html(std::optional<T> const& x)
{
	if(x.has_value())
	{ write_as_html(*x); }
	else
	{ puts("<span class=\"empty\">(no value)</span>"); }
}

template<class T>
void pretty::write_as_html(T const* ptr)
{
	printf("<code class=\"pointer\">%p</code>", ptr);
}

template<class ... T>
void pretty::write_as_html(std::variant<T...> const& val)
{
	std::visit([](auto const& item){ write_as_html(item);}, val);
}

template<class T>
requires(pretty::tuple<T> && !std::ranges::range<T>)
void pretty::write_as_html(T const& x)
{
	if constexpr(elements_have_table_row_formatter<T>())
	{
		if(elements_have_same_size(x))
		{
			puts("<table class=\"tuple_content\">");
			apply_adl([](auto const& ... args){
				(print_table_row(args), ...);
			}, x);
			puts("</table>");
		}
		else
		{
			puts("<ol start=\"0\" class=\"tuple_content\">");
			apply_adl([](auto const&... args){
				(print_list_item(args),...);
			}, x);
			puts("</ol>");
		}
	}
	else
	{
		puts("<ol start=\"0\" class=\"tuple_content\">");
		apply_adl([](auto const&... args){
			(print_list_item(args),...);
		}, x);
		puts("</ol>");
	}
}

template<class T>
void pretty::print_list_item(T const& val)
{
	write_raw("<li>");
	write_as_html(val);
	write_raw("</li>");
}

template<class T>
void pretty::print_table_cell(T const& val)
{
	write_raw("<td>");
	write_as_html(val);
	write_raw("</td>");
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
requires(!std::ranges::forward_range<T>)
void pretty::print_table_row(T const& item)
{
	puts("<tr>");
	apply_adl([](auto const&... args){
		(print_table_cell(args),...);
	}, item);
	puts("</tr>");
}

template<std::ranges::forward_range R>
void pretty::write_as_html(R const& range)
{
	puts("<ol start=\"0\" class=\"range_content\">");
	std::ranges::for_each(range, [](auto const& item){ print_list_item(item); });
	puts("</ol>");
}

template<pretty::fwd_range_of_sized_range R>
void pretty::write_as_html(R const& range)
{
	if constexpr(fwd_range_of_constexpr_sized_range<R>)
	{
		puts("<table class=\"range_content\">");
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

		if constexpr (has_table_row_formatter<std::ranges::range_value_t<R>>)
		{
			if(i == std::end(range))
			{
				puts("<table class=\"range_content\">");
				std::ranges::for_each(range, [](auto const& range){
					print_table_row(range);
				});
				puts("</table>");
			}
		}
		else
		{
			puts("<ol class=\"range_content\" start=\"0\">");
			std::ranges::for_each(range, [](auto const& range) {
				print_list_item(range);
			});
			puts("</ol>");
		}
	}
}

template<class R>
requires(pretty::fwd_range_of_tuple<R> && !pretty::fwd_range_of_sized_range<R>)
void pretty::write_as_html(R const& range)
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

template<class Function, class ... Args>
void pretty::atomic_write(Function&& func, Args&& ... args)
{
	std::lock_guard g{output_mutex};
	func(std::forward<Args>(args)...);
	fflush(stdout);
}

template<class T>
void pretty::print(T const& val)
{
	atomic_write([](auto const& val){
		write_as_html(val);
	}, val);
}

template<class T>
void pretty::print_labeled_value(std::string_view label, T const& value)
{
	atomic_write([](std::string_view label, auto const& value) {
		puts("<table class=\"single_row\">");
		print_table_row(std::tuple{label, "=", value});
		puts("</table>");
	}, label, value);
}

template<class T>
void pretty::print_labeled_value(char const* label, T const& value)
{
	print_labeled_value(std::string_view{label}, value);
}
