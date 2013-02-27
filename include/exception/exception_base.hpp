#ifndef __EXCEPTION_BASE_HPP
#define __EXCEPTION_BASE_HPP

#include <exception>
#include <sstream>
#include <string>
#include <type_traits>

#include "../extend_stl/string/algorithm.hpp"
#include "../win32/debug/stack_walker.hpp"


namespace exception
{
	// ---------------------------------

	namespace detail
	{
		class dump_helper
		{
			std::ostringstream dump_;

		public:
			dump_helper()
			{
				win32::debug::dump_stack(
					[this](void *pvAddress, size_t lineNum, const char * fileName, const char * szModule, const char * szSymbol)
				{
					dump_ << fileName << "(" << lineNum << "): " <<  szSymbol << std::endl;
				}, 
					[](const char *msg)
				{
					assert(0);
				}, 
					5);	// offset frames number
			}

			dump_helper(dump_helper &rhs)
			{
				dump_.swap(rhs.dump_);
			}

		public:
			void dump() const
			{
				::OutputDebugStringA(dump_.str().c_str());
			}
		};
	}
	


	class exception_base
		: public std::exception
#ifdef _DEBUG
		, private detail::dump_helper
#endif
	{
	protected:
		std::string msg_;

	public:
		exception_base()
		{}
		exception_base(const std::string &msg)
			: msg_(msg)
		{}
		virtual ~exception_base()
		{

		}

	protected:
		exception_base(exception_base &rhs)
		{	
			msg_.swap(rhs.msg_);
		}

		exception_base &operator=(const exception_base &);
		

	public:
		template < typename T >
		exception_base &operator<<(const T &val)
		{
			static_assert(std::is_integral<T>::value || 
				std::is_floating_point<T>::value, "T must be a number or char");

			std::string &&tmp = stdex::to_string(val);
			msg_.append(tmp);

			return *this;
		}

		exception_base &operator<<(const char *val)
		{
			msg_.append(val, ::strlen(val));

			return *this;
		}

		exception_base &operator<<(const std::string &val)
		{
			msg_.append(val);

			return *this;
		}

		void dump() const
		{
#ifdef _DEBUG
			detail::dump_helper::dump();
#endif
		}
	public:
		virtual const char *what() const
		{
			return msg_.c_str();
		}
	};


}




#endif