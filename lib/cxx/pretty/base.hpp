#ifndef PRETTY_HPP
#define PRETTY_HPP

#include <string_view>
#include <cstdio>
#include <algorithm>
#include <string>
#include <variant>
#include <limits>
#include <charconv>

namespace pretty
{
	inline void print(char ch);

	template<std::integral T>
	void print(T val);

	template<std::floating_point T>
	void print(T val);

	template<std::integral T>
	void print(T val)
	{
		static constexpr auto num_chars = std::numeric_limits<T>::digits10 + 3;
		std::array<char, num_chars> buffer{};
		std::to_chars(std::data(buffer), std::data(buffer) + std::size(buffer) - 1, val);
		printf("<span class=\"number\">%s</span>", std::data(buffer));
	}

	template<std::floating_point T>
	void print(T val)
	{
		static constexpr auto num_chars = std::numeric_limits<T>::digits10 + 5;
		std::array<char, num_chars> buffer{};
		std::to_chars(std::data(buffer), std::data(buffer) + std::size(buffer) - 1, val);
		printf("<span class=\"number\">%s</span>", std::data(buffer));
	}

	template<class T>
	concept is_pair = requires(T x)
	{
		{x.first};
		{x.second};
	};

	template<std::ranges::input_range R>
	requires(!is_pair<std::ranges::range_value_t<R>>)
	void print(R&& range);

	template<std::ranges::input_range R>
	requires(is_pair<std::ranges::range_value_t<R>>)
	void print(R&& range);

	template<class ... T>
	inline void print(std::variant<T...> const& val);

	void print(char const* c_str)
	{
		print(std::string_view{c_str});
	}

	template<class First, class Second>
	void print(std::pair<First, Second> const& val);

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

	template<std::ranges::input_range R>
	requires(!is_pair<std::ranges::range_value_t<R>>)
	void print(R&& range)
	{
		std::ranges::for_each(range, [](auto const& item){ print(item); });
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

	struct emph
	{
		std::string value;
	};

	inline void print(emph const& em)
	{
		print_raw("<em>");
		print(em.value);
		print_raw("</em>");
	}

	struct link
	{
		std::string href;
		std::string caption;
	};

	inline void print(link const& a)
	{
		print_raw("<a target=\"blank\" href=\"");
		print(a.href);
		print_raw("\">");
		print(a.caption);
		print_raw("</a>");
	}

	struct code
	{
		std::string value;
	};

	inline void print(code const& em)
	{
		print_raw("<code>");
		print(em.value);
		print_raw("</code>");
	}

	struct samp
	{
		std::string value;
	};

	inline void print(samp const& em)
	{
		print_raw("<samp>");
		print(em.value);
		print_raw("</samp>");
	};

	using inline_element = std::variant<std::string_view, emph, link, code, samp>;

	struct paragraph:private std::vector<inline_element>
	{
		using std::vector<inline_element>::vector;

		auto const& get() const
		{ return static_cast<std::vector<inline_element> const&>(*this); }
	};

	inline void print(paragraph const& obj)
	{
		print_raw("<p>");
		print(obj.get());
		print_raw("</p>");
	}

	struct section:private std::vector<inline_element>
	{
		using std::vector<inline_element>::vector;

		auto const& get() const
		{ return static_cast<std::vector<inline_element> const&>(*this); }
	};

	inline void print(section const& obj)
	{
		print_raw("<h3>");
		print(obj.get());
		print_raw("</h3>");
	}

	struct subsection:private std::vector<inline_element>
	{
		using std::vector<inline_element>::vector;

		auto const& get() const
		{ return static_cast<std::vector<inline_element> const&>(*this); }
	};

	inline void print(subsection const& obj)
	{
		print_raw("<h4>");
		print(obj.get());
		print_raw("</h4>");
	}
}

#define PRETTY_PRINT_EXPR(expr) \
pretty::print(std::pair{pretty::code{#expr}, expr})

#endif