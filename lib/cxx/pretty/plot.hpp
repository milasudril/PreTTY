#ifndef PRETTY_PLOT_HPP
#define PRETTY_PLOT_HPP

#include "./base.hpp"

#include <cmath>
#include <span>
#include <cassert>

namespace pretty
{
	template<class T>
	concept arithmetic = std::is_arithmetic_v<T>;

	template<arithmetic X>
	struct plot_axis_range
	{
		X min;
		X max;

		bool empty() const
		{ return min == max; }
	};

	template<arithmetic X>
	auto compute_tick_pitch(plot_axis_range<X> const& range, unsigned int tick_base)
	{
		auto const length = static_cast<double>(range.max - range.min);
		auto const logl = std::round(std::log(length)/std::log(static_cast<double>(tick_base)));
		return std::pow(tick_base, logl - 1.0);
	}

	template<arithmetic X, arithmetic Y>
	struct plot_params_2d
	{
		using x_type = X;
		using y_type = Y;

		std::optional<plot_axis_range<X>> x_range;
		std::optional<plot_axis_range<Y>> y_range;
		unsigned int x_tick_base = 5;
		unsigned int y_tick_base = 5;
	};

	template<class T>
	concept plot_point_2d = tuple<T> && requires(T x)
	{
		{get<0>(x)} -> std::convertible_to<arithmetic>;
		{get<1>(x)} -> std::convertible_to<arithmetic>;
	};

	template<class T>
	concept plot_data_2d = std::ranges::forward_range<T> && plot_point_2d<std::ranges::range_value_t<T>>;

	template<plot_data_2d T>
	struct plot_2d_coord_types
	{
		using plot_point_type = std::ranges::range_value_t<T>;
		using x_type = std::decay_t<decltype(get<0>(std::declval<plot_point_type>()))>;
		using y_type = std::decay_t<decltype(get<1>(std::declval<plot_point_type>()))>;
	};

	template<plot_data_2d T>
	struct make_plot_params_2d
	{
	public:
		using type = plot_params_2d<typename plot_2d_coord_types<T>::x_type,
			typename plot_2d_coord_types<T>::y_type>;
	};

	template<plot_data_2d T>
	using plot_params_2d_t = make_plot_params_2d<T>::type;

	template<size_t Element, plot_data_2d PlotData>
	auto compute_range(PlotData const& data)
	{
		assert(std::begin(data) != std::end(data));

		auto minmax = std::ranges::minmax_element(data, [](auto const& a, auto const& b) {
			return get<Element>(a) < get<Element>(b);
		});

		return plot_axis_range{get<Element>(*minmax.min), get<Element>(*minmax.max)};
	}

	template<size_t Element, std::ranges::forward_range R>
	requires(plot_data_2d<std::ranges::range_value_t<R>>)
	auto compute_range(R const& plot_data_range)
	{
		auto current = std::begin(plot_data_range);
		auto const end = std::end(plot_data_range);
		assert(current != end);

		auto ret = compute_range<Element>(*current);
		++current;
		while(current != std::end(plot_data_range))
		{
			auto const tmp = compute_range<Element>(*current);
			ret = plot_axis_range{std::min(tmp.min, ret.min), std::max(tmp.max, ret.max)};
			++current;
		}

		return ret;
	}

	template<arithmetic X, class Callable>
	void in_steps(plot_axis_range<X> range, double dx, Callable&& func)
	{
		auto const x0 = std::ceil(range.min/dx)*dx;
		size_t k = 0;
		while(true)
		{
			auto const x = x0 + static_cast<double>(k)*dx;
			if(x > range.max)
			{ return; }

			func(k, x);
			++k;
		}
	}

	inline constexpr std::string_view curve_ids{"0123456789abcdef"};
	static_assert(std::size(curve_ids) == 16);

	template<std::ranges::forward_range R>
	requires(plot_data_2d<std::ranges::range_value_t<R>>)
	class plot_context_2d
	{
	public:
		using PlotData = std::decay_t<std::ranges::range_value_t<R>>;

		explicit plot_context_2d(R const& plot_data,
			plot_params_2d_t<PlotData> const& plot_params):
			m_plot_data{plot_data},
			m_x_range{plot_params.x_range.value_or(compute_range<0>(plot_data))},
			m_y_range{plot_params.x_range.value_or(compute_range<1>(plot_data))}
		{
			assert(!m_x_range.empty());
			assert(!m_y_range.empty());

			auto const w = m_x_range.max - m_x_range.min;
			auto const h = m_y_range.max - m_y_range.min;
			m_w = static_cast<double>(w);
			m_h = static_cast<double>(h);

			m_scale = 512.0/std::max(m_w, m_h);
			m_w *= m_scale;
			m_h *= m_scale;
			m_sx_range = plot_axis_range{m_scale*static_cast<double>(m_x_range.min),
				m_scale*static_cast<double>(m_x_range.max)};
			m_sy_range = plot_axis_range{m_scale*static_cast<double>(m_y_range.min),
				m_scale*static_cast<double>(m_y_range.max)};

			m_x_tick_pitch = compute_tick_pitch(m_x_range, plot_params.x_tick_base);
			m_y_tick_pitch = compute_tick_pitch(m_y_range, plot_params.y_tick_base);

			m_x_min_chars = to_char_buffer(m_sx_range.min);
			m_x_max_chars = to_char_buffer(m_sx_range.max);
			m_y_min_chars = to_char_buffer(m_sy_range.min);
			m_y_max_chars = to_char_buffer(m_sy_range.max);
		}

		void operator()() const
		{
			constexpr auto text_height = 16;
			write_raw("<svg viewbox=\"");
			write_raw(std::data(to_char_buffer(m_sx_range.min - 4*text_height)));
			putchar(' ');
			write_raw(std::data(to_char_buffer(m_sy_range.min - text_height)));
			putchar(' ');
			write_raw(std::data(to_char_buffer(m_w + 8*text_height)));
			putchar(' ');
			write_raw(std::data(to_char_buffer(m_h + 2.5*text_height)));
			puts("\">");

			auto const print_coord = [scale = m_scale, y_range = m_y_range](auto const& item) {
				auto const x = scale*get<0>(item);
				auto const y = scale*(y_range.max + y_range.min - get<1>(item));
				write_raw(std::data(to_char_buffer(x)));
				putchar(',');
				write_raw(std::data(to_char_buffer(y)));
				putchar(' ');
			};
#if 1
//			TODO
			auto const draw_marker = [scale = m_scale, y_range = m_y_range](auto const& item) {
				auto const x = scale*get<0>(item);
				auto const y = scale*(y_range.max + y_range.min - get<1>(item));
				write_raw("<circle cx=\"");
				write_raw(std::data(to_char_buffer(x)));
				write_raw("\" cy=\"");
				write_raw(std::data(to_char_buffer(y)));
				puts("\" r=\"2\" fill=\"blue\" stroke=\"none\"/>");
			};
#endif

			std::ranges::for_each(m_plot_data.get(), [k = static_cast<size_t>(0), &print_coord, &draw_marker]
				(auto const& curve) mutable {
				write_raw("<polyline class=\"curve_");
				putchar(curve_ids[k%std::size(curve_ids)]);
				puts("\" stroke=\"blue\" stroke-width=\"1\" fill=\"none\" points=\"");
				std::ranges::for_each(curve, print_coord);
				++k;
				puts("\"/>");

				std::ranges::for_each(curve, draw_marker);
			});

			// Draw x grid
			in_steps(m_x_range, m_x_tick_pitch,
				[scale = m_scale, y_min_chars = std::data(m_y_min_chars), y_max_chars = std::data(m_y_max_chars)]
				(auto, double x) {
				puts("<polyline class=\"x_grid\" stroke-width=\"1\" fill=\"none\" points=\"");
				auto const xbuff = to_char_buffer(scale*x);
				write_raw(std::data(xbuff));
				putchar(',');
				write_raw(y_min_chars);
				putchar(' ');
				write_raw(std::data(xbuff));
				putchar(',');
				write_raw(y_max_chars);
				puts("\"/>");
			});

			// Draw y grid
			in_steps(m_y_range, m_y_tick_pitch,
				[scale = m_scale,
					x_min_chars = std::data(m_x_min_chars),
					x_max_chars = std::data(m_x_max_chars),
					y_range=m_y_range]
				(auto, double y) {
				write_raw("<polyline class=\"y_grid\" stroke-width=\"1\" fill=\"none\" points=\"");
				auto const ybuff = to_char_buffer(scale*(y_range.max + y_range.min - y));
				write_raw(x_min_chars);
				putchar(',');
				write_raw(std::data(ybuff));
				putchar(' ');
				write_raw(x_max_chars);
				putchar(',');
				write_raw(std::data(ybuff));
				puts("\"/>");
			});

			// Draw x labels
			in_steps(m_x_range, m_x_tick_pitch,
				[scale = m_scale, y_max_chars = std::data(m_y_max_chars)](auto, double x) {
				write_raw("<text class=\"x_labels\" style=\"font-size:");
				write_raw(std::data(to_char_buffer(text_height)));
				write_raw("px\" text-anchor=\"middle\" dominant-baseline=\"hanging\" x=\"");
				write_raw(std::data(to_char_buffer(scale*x)));
				write_raw("\" y=\"");
				write_raw(y_max_chars);
				write_raw("\">");
				write_raw(std::data(to_char_buffer(static_cast<float>(x))));
				puts("</text>");
			});

			// Draw y labels
			in_steps(m_y_range, m_y_tick_pitch,
				[scale = m_scale,
					x_loc_chars = to_char_buffer(m_sx_range.min - 2),
					y_min_chars = std::data(m_y_min_chars),
					y_max_chars = std::data(m_y_max_chars),
					y_range=m_y_range]
				(auto, double y) {
				write_raw("<text class=\"x_labels\" style=\"font-size:");
				write_raw(std::data(to_char_buffer(text_height)));
				write_raw("px\" text-anchor=\"end\" dominant-baseline=\"middle\" x=\"");
				write_raw(std::data(x_loc_chars));
				write_raw("\" y=\"");
				auto const ybuff = to_char_buffer(scale*(y_range.max + y_range.min - y));
				write_raw(std::data(ybuff));
				write_raw("\">");
				write_raw(std::data(to_char_buffer(static_cast<float>(y))));
				puts("</text>");
			});

			write_raw("<rect class=\"axis_box\" fill=\"none\" stroke-width=\"1\" x=\"");
			write_raw(std::data(m_x_min_chars));
			write_raw("\" y=\"");
			write_raw(std::data(m_y_min_chars));
			write_raw("\" width=\"");
			write_raw(std::data(to_char_buffer(m_w)));
			write_raw("\" height=\"");
			write_raw(std::data(to_char_buffer(m_h)));
			puts("\"/>");
			puts("</svg>");
		}

	private:
		std::reference_wrapper<R const> m_plot_data;

		using x_type = typename plot_2d_coord_types<PlotData>::x_type;
		using y_type = typename plot_2d_coord_types<PlotData>::y_type;

		plot_axis_range<x_type> m_x_range;
		plot_axis_range<y_type> m_y_range;
		double m_scale;
		double m_w;
		double m_h;
		plot_axis_range<double> m_sx_range;
		plot_axis_range<double> m_sy_range;

		double m_x_tick_pitch;
		double m_y_tick_pitch;

		std::array<char, 32> m_x_min_chars;
		std::array<char, 32> m_x_max_chars;
		std::array<char, 32> m_y_min_chars;
		std::array<char, 32> m_y_max_chars;
	};

	template<plot_data_2d PlotData>
	void plot(PlotData const& plot_data,
		plot_params_2d_t<PlotData> const& plot_params = plot_params_2d_t<PlotData>{})
	{
		atomic_write(plot_context_2d{std::span<PlotData const, 1>{&plot_data, 1}, plot_params});
	}

	template<std::ranges::forward_range R>
	requires(plot_data_2d<std::ranges::range_value_t<R>>)
	void plot(R const& plot_data,
		plot_params_2d_t<std::ranges::range_value_t<R>> const& plot_params = plot_params_2d_t<std::ranges::range_value_t<R>>{})
	{
		atomic_write(plot_context_2d{plot_data, plot_params});
	}
}

#endif
