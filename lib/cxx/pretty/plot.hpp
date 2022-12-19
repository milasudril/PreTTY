#ifndef PRETTY_PLOT_HPP
#define PRETTY_PLOT_HPP

#include "./base.hpp"

namespace pretty
{
	template<fwd_range_of_tuple R>
	void plot(R const& range)
	{
		atomic_write([](auto const& range){
			puts("<div style=\"width:80%; aspect-ratio:16/10; border:1px solid; margin-left:auto; margin-right:auto\">");
			puts("<svg viewbox=\"-1024 -1024 2048 2048\" width=\"100%\" height=\"100%\">");
			puts("<polyline stroke-width=\"4\" stroke=\"blue\" points=\"");
			std::ranges::for_each(range, [](auto const& item) {
				printf("%.8e,%.8e ", 1024.0f*get<0>(item), -1024.0f*get<1>(item));
			});
			puts("\" fill=\"none\"/>");
			puts("</svg>");
			puts("</div>");
		}, range);
	}
}

#endif