#include <pretty/plot.hpp>

#include <geosimd/euclidian_space.hpp>

using my_point = geosimd::euclidian_space<float, 3>::location;
using my_vector = geosimd::euclidian_space<float, 3>::displacement;

int main()
{
	constexpr auto N = 128;
	std::array<my_point, 12*N + 1> vals{};
	for(int32_t k = 0; k != 12*N + 1; ++k)
	{
		auto const theta = geosimd::rotation_angle{geosimd::rad{static_cast<double>(k)/N}};
		auto cs = cossin(theta);
		vals[k] = my_point{} + my_vector{static_cast<float>(k)/N, cs.sin, 0.0f};
	}

	pretty::plot(vals);
}