#include <pretty/plot.hpp>
#include <pretty/annotations.hpp>

#include <geosimd/euclidian_space.hpp>

using my_point = geosimd::euclidian_space<float, 3>::location;
using my_vector = geosimd::euclidian_space<float, 3>::displacement;

int main()
{
	constexpr auto N = 24;
	pretty::paragraph("PreTTY can be used to produce simple function graphs, like in the figure below:");

	std::array<my_point, 2*N + 1> vals{};
	for(int32_t k = -24; k != 2*N + 1 -24; ++k)
	{
		auto const theta = geosimd::rotation_angle{geosimd::turns{static_cast<double>(k)/(4.0*N)}};
		auto cs = cossin(theta);
		vals[k+24] = my_point{0.0f, 0.0f, 0.0f} + my_vector{static_cast<float>(k)/N, cs.cos, 0.0f};
	}

	pretty::figure fig{"cos(2πx) on the interval [-1, 1]"};
	pretty::plot(vals);
}