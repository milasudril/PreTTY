#ifndef PRETTY_ANNOTATIONS_HPP
#define PRETTY_ANNOTATIONS_HPP

#include "./base.hpp"

namespace pretty
{
	template<class Content>
	inline void paragraph(Content const& content)
	{
		atomic_write([](Content const& content){
			write_raw("<p>");
			write_as_html(content);
			write_raw("</p>");
		}, content);
	}

	template<class Caption>
	inline void section(Caption const& caption)
	{
		atomic_write([](Caption const& caption){
			write_raw("<h3>");
			write_as_html(caption);
			write_raw("</h3>");
		}, caption);
	}

	template<class Caption>
	inline void subsection(Caption const& caption)
	{
		atomic_write([](Caption const& caption){
			write_raw("<h4>");
			write_as_html(caption);
			write_raw("</h4>");
		}, caption);
	}

	template<class Caption>
	inline void subsubsection(Caption const& caption)
	{
		atomic_write([](Caption const& caption){
			write_raw("<h5>");
			write_as_html(caption);
			write_raw("</h5>");
		}, caption);
	}


	struct box
	{
		[[nodiscard]] box()
		{
			write_raw("<div class=\"box\">");
		}

		~box()
		{
			write_raw("</div>");
			fflush(stdout);
		}

		std::shared_lock<std::shared_mutex> lock{output_mutex};
	};
}
#endif