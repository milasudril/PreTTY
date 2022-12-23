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
			puts("<div class=\"box\">");
		}

		~box()
		{
			puts("</div>");
			fflush(stdout);
		}

		std::lock_guard<std::recursive_mutex> lock{output_mutex};
	};
	
	template<class Caption>
	class figure
	{
	public:
		[[nodiscard]] explicit figure(Caption caption):
			m_caption{caption}
		{
			write_raw("<figure>");
		}
		
		~figure()
		{
			write_as_html(m_caption);
			puts("</figure>");
			fflush(stdout);
		}
		
	private:
		Caption m_caption;
	};
}
#endif
