#include <pretty/plot.hpp>
#include <pretty/annotations.hpp>

#include <geosimd/euclidian_space.hpp>

using my_point = geosimd::euclidian_space<float, 3>::location;
using my_vector = geosimd::euclidian_space<float, 3>::displacement;

int main()
{
	constexpr auto N = 24;
	pretty::paragraph("PreTTY can be used to produce simple function graphs, like in the figure below:");

	std::array<my_point, 4*N + 1> cos{};
	std::array<my_point, 4*N + 1> sin{};
	for(int32_t k = -48; k != 4*N + 1 - 48; ++k)
	{
		auto const theta = geosimd::rotation_angle{geosimd::turns{static_cast<double>(k)/(1.0*N)}};
		auto cs = cossin(theta);
		cos[k + 48] = my_point{0.0f, 0.0f, 0.0f} + my_vector{static_cast<float>(k)/N, 2.0f*cs.cos, 0.0f};
		sin[k + 48] = my_point{0.0f, 0.0f, 0.0f} + my_vector{static_cast<float>(k)/N + 1.0f, cs.sin, 0.0f};
	}

	pretty::figure fig{"cos(2πx) and sin(2πx) on the interval [-2, 2] and [-1, 3], respectively"};
	pretty::plot(std::initializer_list{cos, sin});
}