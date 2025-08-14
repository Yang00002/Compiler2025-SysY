#ifndef UTIL_HPP
#define UTIL_HPP
#include <cassert>

#define ifThenOrThrow(cond, ifCondThen, Message) ASSERT(!(cond)  || (ifCondThen))

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG == 1
#include <iostream>
#include "Color.hpp"
namespace
{
	namespace debug
	{
		std::string tab_counts;

		void _0__counts_pop()
		{
			if (!tab_counts.empty()) tab_counts.pop_back();
		}

		void _0__counts_push()
		{
			tab_counts += ' ';
		}

		void _0__log_if_func(const std::string& str, const bool cond)
		{
			if (cond)
				std::cout << tab_counts << str << '\n';
		}

		void _0__log_func(const std::string& str)
		{
			std::cout << tab_counts << str << '\n';
		}

		void _0__gap_func()
		{
			std::cout << "==============================\n";
		}
	}
}

#define LOG(a) debug::_0__log_func(a)
#define LOGIF(a,b) debug::_0__log_if_func((a), (b))
#define GAP debug::_0__gap_func()
#define PUSH debug::_0__counts_push()
#define RUN(a) (a)
#define POP debug::_0__counts_pop()
#define PASS_SUFFIX LOG(color::green("Getting:")); \
GAP; \
LOG(m_->print()); \
GAP
#define PREPARE_PASS_MSG m_->set_print_name()
#endif
#if DEBUG != 1
#define LOG(a)
#define LOGIF(a,b)
#define GAP
#define RUN(a)
#define PUSH
#define POP
#define PASS_SUFFIX
#define PREPARE_PASS_MSG
#endif
#endif


#ifndef OPEN_ASSERT
#define OPEN_ASSERT 0
#endif

#if OPEN_ASSERT == 1
#define ASSERT(a) assert(a)
#endif
#if OPEN_ASSERT != 1
#define ASSERT(a) 
#endif
