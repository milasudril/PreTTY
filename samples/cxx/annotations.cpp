#include <pretty/base.hpp>
#include <pretty/annotations.hpp>

#include <ctime>

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
		the value of it. It is like assert, but with only the print-out)");

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
}