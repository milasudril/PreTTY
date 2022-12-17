#include <pretty/base.hpp>
#include <pretty/annotations.hpp>

#include <numbers>
#include <map>
#include <vector>

int main()
{
	print(pretty::section{"Hello, World"});
	print(pretty::paragraph{"This paragraph is formatted using ",
		pretty::code{"PreTTY"}, ". When formatting text with PreTTY it is possible to usecan use ",
		pretty::code{"code"}, ", ", pretty::emph{"emph"}, ", and ",
		pretty::link{.href="https://en.wikipedia.org/wiki/Hyperlink", .caption="hyperlinks"}, "."
	});

	print(pretty::subsection{"Printing content of std containers"});
	print(pretty::paragraph{"Any container can be printed as so:"});

	std::array&lt;int, 5&gt; some_ints{1, 2, 3, 4, 5};
	pretty::print(some_ints);

	print(pretty::paragraph{"An array of floats"});

	std::array&lt;float, 5&gt; some_floats{0.0f,
		1.0f,
 		std::numbers::phi_v&lt;float&gt;,
		std::numbers::e_v&lt;float&gt;,
		std::numbers::pi_v&lt;float&gt;};
	pretty::print(some_floats);

	print(pretty::paragraph{"Double gives some more digits"});
	std::array&lt;double, 5&gt; some_doubles{0.0,
		1.0,
 		std::numbers::phi_v&lt;double&gt;,
		std::numbers::e_v&lt;double&gt;,
		std::numbers::pi_v&lt;double&gt;};
	pretty::print(some_doubles);

	print(pretty::paragraph{"A pair is written as a table"});
	pretty::print(std::pair{"π", std::numbers::pi_v&lt;double&gt;});

	print(pretty::paragraph{"This is useful in case there is a long description:"});
	pretty::print(std::pair{"π", "The number π (/paɪ/; spelled out as \"pi\") is a mathematical constant that is the ratio of a circle's circumference to its diameter, approximately equal to 3.14159."});

	print(pretty::paragraph{"As pairs, any container whose value_type resolves to a pair is printed as a table:"});

	using map_types = std::variant&lt;std::map&lt;std::string, int&gt;, std::vector&lt;std::pair&lt;std::string, int&gt;&gt;&gt;;

	pretty::print(
		std::map&lt;std::string, map_types&gt;
		{
			{"A map", std::map&lt;std::string, int&gt;{{"Key 1", 12}, {"Key 2", 45}}},
			{"An vector of pairs", std::vector&lt;std::pair&lt;std::string, int&gt;&gt;{{"Key 3", 212}, {"Key 4", 145}}}
		}
	);

	print(pretty::paragraph{"The example above also shows that it is possible to use nested containers"});

	print(pretty::paragraph{"An optional may have a value"});

	pretty::print(std::optional{"This optional has value"});

	print(pretty::paragraph{"or not"});

	pretty::print(std::optional&lt;std::string_view&gt;{});

	print(pretty::paragraph{"A tuple is printed similarly to an ordered container"});

	pretty::print(std::tuple{"Foo", "bar", 1.3, 1234});

	print(pretty::subsection{"\"Debug\" output"});

	print(pretty::paragraph{"Use ", pretty::code{"PRETTY_PRINT_EXPR"}, " to print the value of an expression:"});

	PRETTY_PRINT_EXPR(4*5);
}