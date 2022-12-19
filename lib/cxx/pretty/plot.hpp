#ifndef PRETTY_PLOT_HPP
#define PRETTY_PLOT_HPP

#include "./base.hpp"

namespace pretty
{
	template<class X>
	requires(std::is_arithmetic_v<X>)
	struct plot_axis_range
	{
		X min;
		X max;
	};

	struct plot_line_style
	{};

	template<class X, class Y>
	requires(std::is_arithmetic_v<X> && std::is_arithmetic_v<Y>)
	struct plot_params
	{
		std::optional<plot_axis_range<X>> x_range;
		std::optional<plot_axis_range<Y>> y_range;
		plot_line_style line_style;
		uint32_t x_tick_base = 10;
		uint32_t y_tick_base = 10;
		std::optional<plot_line_style> x_axis;
		std::optional<plot_line_style> y_tick_lins;
		std::optional<plot_line_style> y_axis;
		std::optional<plot_line_style> x_tick_lines;
	};

	template<fwd_range_of_tuple R>
	void plot(R const& range)
	{
		atomic_write([](auto const& range){
			puts("<div style=\"width:80%; aspect-ratio:16/10; border:1px solid; margin-left:auto; margin-right:auto\">");
			puts("<svg viewbox=\"-1024 -1024 2048 2048\" width=\"100%\" height=\"100%\">");
			puts("<polyline stroke-width=\"4\" stroke=\"blue\" points=\"");
			std::ranges::for_each(range, [](auto const& item) {
				auto const x =  1024.0f*get<0>(item);
				auto const y = -1024.0f*get<1>(item);
				write_raw(std::data(to_char_buffer(x)));
				putchar(',');
				write_raw(std::data(to_char_buffer(y)));
				putchar(' ');
			});
			puts("\" fill=\"none\"/>");
			puts("</svg>");
			puts("</div>");
		}, range);
	}
}

#endif