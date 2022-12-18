#include <pretty/plot.hpp>

#include <geosimd/euclidian_space.hpp>

using my_point = geosimd::euclidian_space<float, 3>::location;
using my_vector = geosimd::euclidian_space<float, 3>::displacement;

int main()
{
	std::array<my_point, 12> vals{};
	for(size_t k = 0; k != 12; ++k)
	{
		auto const theta = geosimd::rotation_angle{geosimd::turns{static_cast<double>(k)/12}};
		auto cs = cossin(theta);
		vals[k] = my_point{} + 1.0f*my_vector{cs.cos, cs.sin, 0.0f};
	}

	pretty::plot_as_svg(vals);
}