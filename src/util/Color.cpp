#include "Color.hpp"
#include <iostream>
#include <string>
#include <cassert>
using std::ostream;
using std::wstring;

namespace color
{
	bool Color::inrange(const int& num)
	{
		return num > -1 && num < 256;
	}

	Color::Color()
	{
		colorstr = "\033[0m";
	}

	Color::Color(const fore& f, const Color::back& b, const Color::kind& k)
	{
		colorstr = "\033[" + std::to_string(static_cast<int>(k)) + ";" + std::to_string(static_cast<int>(f)) + ";" +
		           std::to_string(static_cast<int>(b)) + "m";
	}

	Color::Color(const int& r1, const int& g1, const int& b1, const int& r2, const int& g2, const int& b2,
	             const kind& k)
	{
		assert(inrange(r1) && inrange(g1) && inrange(b1) && inrange(r2) && inrange(g2) && inrange(b2));
		colorstr = "\033[" + std::to_string(static_cast<int>(k)) + ";38;2;" + std::to_string(r1) + ";" +
		           std::to_string(g1) + ";" + std::to_string(b1) + ";48;2;" + std::to_string(r2) + ";" +
		           std::to_string(g2) + ";" + std::to_string(b2) + "m";
	}

	Color::Color(const fore& f, const int& r2, const int& g2, const int& b2, const kind& k)
	{
		assert(inrange(r2) && inrange(g2) && inrange(b2));
		colorstr = "\033[" + std::to_string(static_cast<int>(k)) + ";" + std::to_string(static_cast<int>(f)) + ";48;2;"
		           + std::to_string(r2) + ";" + std::to_string(g2) + ";" + std::to_string(b2) + "m";
	}

	Color::Color(const int& r1, const int& g1, const int& b1, const back& b, const kind& k)
	{
		assert(inrange(r1) && inrange(g1) && inrange(b1));
		colorstr = "\033[" + std::to_string(static_cast<int>(k)) + ";38;2;" + std::to_string(r1) + ";" +
		           std::to_string(g1) + ";" + std::to_string(b1) + ";" + std::to_string(static_cast<int>(b)) + "m";
	}

	Color::operator std::string()
	{
		return this->colorstr;
	}

	std::string Color::operator()(const std::string& s) const
	{
		return this->colorstr + s + "\033[0m";
	}

	std::string Color::operator()(const char* s) const
	{
		return this->colorstr + s + "\033[0m";
	}

	Color black(Color::fore::black);
	Color red(Color::fore::red);
	Color green(Color::fore::green);
	Color yellow(Color::fore::yellow);
	Color blue(Color::fore::blue);
	Color pink(Color::fore::pink);
	Color cyan(Color::fore::cyan);
	Color white(Color::fore::white);
	Color reset(Color::fore::none);

	std::ostream& operator<<(std::ostream& os, const Color& co)
	{
		os << co.colorstr;
		return os;
	}
}
