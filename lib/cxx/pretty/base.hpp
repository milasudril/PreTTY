#ifndef PRETTY_HPP
#define PRETTY_HPP

#include <string_view>
#include <cstdio>
#include <algorithm>

namespace pretty
{
	inline void print(char ch)
	{
		switch(ch)
		{
		case '&':
			printf("&amp;");
			break;
		case '<':
			printf("&lt;");
			break;
		case '>':
			printf("&gt;");
			break;
		case '"':
			printf("&quot;");
			break;

		default:
			putchar(ch);
		}
	}

	inline void print(std::string_view str)
	{
		std::ranges::for_each(str, [](auto item){print(item);});
	}

	inline void print_raw(std::string_view str)
	{
		std::ranges::for_each(str, [](auto item){putchar(item);});
	}
}

#endif