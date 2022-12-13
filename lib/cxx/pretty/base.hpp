#ifndef PRETTY_HPP
#define PRETTY_HPP

#include <string_view>
#include <cstdio>
#include <algorithm>
#include <string>
#include <variant>

namespace pretty
{
	inline void print(char ch);

	template<std::ranges::input_range R, class ... PrintParams>
	void print(R&& range, PrintParams&&... params)
	{
		std::ranges::for_each(range, [...params=std::forward<PrintParams>(params)](auto const& item){
			print(item, params...);
		});
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

	template<class ... T, class ... PrintArgs>
	inline void print(std::variant<T...> const& val, PrintArgs&& ... args)
	{
		std::visit([...params=std::forward<PrintArgs>(args)](auto const& item){
			print(item, params...);
		}, val);
	}

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

#endif