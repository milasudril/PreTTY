#include <pretty/base.hpp>
#include <pretty/annotations.hpp>

#include <ctime>
#include <map>
#include <cstdint>
#include <numbers>

extern char** environ;

int main()
{
	pretty::paragraph(R"(PreTTY is a tool for quick preview of the results from programs that
		generate so-called rich content. This includes things like images and audio, that cannot
		be easily represented as text. It does so by presenting the result on a generated web
		page. Output is generated through a language-specific support library. To avoid having
		to write data to a web page on disk, PreTTY is build as a web application, with the
		compiler and tested application running on the server.)");

	pretty::section("Using the UI");
	pretty::paragraph(R"(The program source is written in the left panel. To run the program,
		click "Build and run". It is also possible to use one of the keyboard shortcuts
		CTRL + Enter or CTRL + →. The latter also switches view so only the output of the
		program is visible. Use CTRL + ← to move focus to the source code.)");

	pretty::section("Annotations");
	pretty::paragraph(R"(To make it easier to follow the output, it is possible to add annotations
		to the program output. This includes section titles and describing text. For C++, the first
		feature is the PRETTY_PRINT_EXPR macro, which expands stringifies its argument and prints
		the value of it. It is like assert, but with only the print-out.)");

	PRETTY_PRINT_EXPR(time(nullptr));

	pretty::paragraph(R"(The annotations module provides functions that wraps their argument in an
		appropriate HTML element. There is paragraph, section, subsection, and subsubsection. Content
		can also be wrapped inside a box:)");

	{
		pretty::box a_box{};
		pretty::section("This is a section title");
		pretty::subsection("This is a subsection");
		pretty::subsubsection("This is a subsubsection");
		pretty::paragraph("Use paragraphs for descriptive text");
	}

	pretty::section("Formatting of types using known protocols");
	pretty::paragraph(R"(The following table shows how the C++ PreTTY support library formats data types
		that adheres to different protocols)");

	pretty::print(std::tuple{
		std::pair{"A single character", 'A'},
		std::pair{"A single byte", std::byte{65}},
		std::pair{"A signed 8-bit-integer", 65},
		std::pair{"A unsigned 8-bit-integer", 156},
		std::pair{"A signed 64-bit-integer", -6590486094},
		std::pair{"A float", std::numbers::pi_v<float>},
		std::pair{"A double", std::numbers::pi_v<double>},
		std::pair{"A large value", 6.02214076e23},
		std::pair{"A small value", 6.62607015e-34},
		std::pair{"A string", "HTML characters are always escaped: & < > \""},
		std::pair{"An optional with value", std::optional{123}},
		std::pair{"An optional without value", std::optional<int>{}},
		std::pair{"A pointer", environ}
	});
}
