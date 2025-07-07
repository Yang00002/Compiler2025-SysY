#pragma once
#include <string>
#include <ostream>

namespace color
{
	class Color
	{
		std::string colorstr;
		static bool inrange(const int& num);

	public:
		enum class kind : unsigned char { none = 10, underline = 4 };

		enum class fore : unsigned char
		{
			none = 39, black = 30, red = 31, green = 32, yellow = 33, blue = 34, pink = 35, cyan = 36, white = 37
		};

		enum class back : unsigned char
		{
			none = 49, black = 40, red = 41, green = 42, yellow = 43, blue = 44, pink = 45, cyan = 46, white = 47
		};

		Color();
		Color(const enum fore& f, const enum back& b = back::none, const enum kind& k = kind::none);
		Color(const int& r1, const int& g1, const int& b1, const enum back& b = back::none,
		      const enum kind& k = kind::none);
		Color(const enum fore& f, const int& r2, const int& g2, const int& b2, const enum kind& k = kind::none);
		Color(const int& r1, const int& g1, const int& b1, const int& r2, const int& g2, const int& b2,
		      const enum kind& k = kind::none);
		friend std::ostream& operator<<(std::ostream& os, const Color& co);
		friend std::wostream& operator<<(std::wostream& os, const Color& co);
		operator std::string();

		template <typename T>
		std::string operator()(T s) const
		{
			return this->colorstr + std::to_string(s) + "\033[0m";
		}

		std::string operator()(const std::string& s) const;
		std::string operator()(const char* s) const;
	};

	extern Color black;
	extern Color red;
	extern Color green;
	extern Color yellow;
	extern Color blue;
	extern Color pink;
	extern Color cyan;
	extern Color white;
	extern Color reset;
}
