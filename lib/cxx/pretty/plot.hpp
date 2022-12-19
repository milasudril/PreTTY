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
	{
		float line_width = 1.0f;
	};

	template<class T>
	concept arithmetic = std::is_arithmetic_v<T>;

	template<arithmetic X, arithmetic Y>
	struct plot_params_2d
	{
		using x_type = X;
		using y_type = Y;

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

	template<class T>
	concept plot_point_2d = tuple<T> && requires(T x)
	{
		{get<0>(x)} -> arithmetic;
		{get<1>(x)} -> arithmetic;
	};

	template<class T>
	concept plot_data_2d = std::ranges::input_range<T> && plot_point_2d<std::ranges::range_value_t<T>>;

	template<plot_data_2d T>
	struct make_plot_params_2d
	{
	private:
		using plot_point_type = std::ranges::range_value_t<T>;
		using x_type = std::decay_t<decltype(get<0>(std::declval<plot_point_type>()))>;
		using y_type = std::decay_t<decltype(get<1>(std::declval<plot_point_type>()))>;

	public:
		using type = plot_params_2d<x_type, y_type>;
	};

	template<plot_data_2d T>
	using plot_params_2d_t = make_plot_params_2d<T>::type;

	template<plot_data_2d PlotData>
	void plot(PlotData const& plot_data,
		plot_params_2d_t<PlotData> const& = plot_params_2d_t<PlotData>{})
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
		}, plot_data);
	}
}

#endif