#ifndef PRETTY_ANNOTATIONS_HPP
#define PRETTY_ANNOTATIONS_HPP

#include "./base.hpp"

namespace pretty
{
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
		std::ranges::for_each(obj.get(), [](auto const& item) {
			print(item);
		});
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
		std::ranges::for_each(obj.get(), [](auto const& item) {
			print(item);
		});
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
		std::ranges::for_each(obj.get(), [](auto const& item) {
			print(item);
		});
		print_raw("</h4>");
	}
}
#endif