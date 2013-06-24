#ifndef __EXCEPTION_BASE_HPP
#define __EXCEPTION_BASE_HPP


/** @exception_base.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/08>
* @version <0.1>
*
* 异常基础类，
*	debug模式下，会输出抛出异常的调用堆栈
*	release模式下，则不会输出堆栈
* 你可以通过调试窗口查看堆栈信息
*/

#include <windows.h>
#include <exception>
#include <sstream>
#include <string>
#include <type_traits>
#include <system_error>

#include "../extend_stl/string/algorithm.hpp"	// for stdex::to_string
#include "../win32/debug/stack_walker.hpp"		// for win32::debug::dump_stack


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

		
		struct dump_null
		{
			void dump() const
			{}
		};
	}
	

	/**
	* @class <exception_base_t>
	* @brief 异常基础类，当抛出异常时在调试窗口打印堆栈信息
	*
	* DumpT dump policy
	*	detail::dump_helper	debug模式使用
	*	detail::dump_null	release模式使用
	*/

	template < typename DumpT >
	class exception_base_t
		: public std::exception
		, private DumpT
	{
	protected:
		std::error_code code_;
		std::string msg_;

	public:
		exception_base_t(std::error_code code, const std::string &msg)
			: msg_(msg)
			, code_(code)
		{}
		virtual ~exception_base_t()
		{

		}

	protected:
		exception_base_t(exception_base_t &rhs)
		{	
			msg_.swap(rhs.msg_);
		}

		exception_base_t &operator=(const exception_base_t &);
		

	public:
		/**
		* @brief 追加参数信息
		* @param <val> <整型或浮点类型值>
		* @exception <不会抛出任何异常>
		* @return <exception_base_t &>
		* @note <输入val必须为整型或浮点类型值>
		* @remarks <>
		*/
		template < typename T >
		exception_base_t &operator<<(const T &val)
		{
			static_assert(std::is_integral<T>::value || 
				std::is_floating_point<T>::value, "T must be a number or char");

			std::string &&tmp = stdex::to_string(val);
			msg_.append(tmp);

			return *this;
		}

		/**
		* @brief 追加参数信息
		* @param <val> <ascii字符串>
		* @exception <不会抛出任何异常>
		* @return <exception_base_t &>
		* @note <>
		* @remarks <>
		*/
		exception_base_t &operator<<(const char *val)
		{
			msg_.append(val, ::strlen(val));

			return *this;
		}

		/**
		* @brief 追加参数信息
		* @param <val> <std::string字符串>
		* @exception <不会抛出任何异常>
		* @return <exception_base_t &>
		* @note <>
		* @remarks <>
		*/
		exception_base_t &operator<<(const std::string &val)
		{
			msg_.append(val);

			return *this;
		}

		/**
		* @brief 捕获异常后再调试窗口打印堆栈信息
		* @param <>
		* @exception <不会抛出任何异常>
		* @return <>
		* @note <在捕获异常后调用>
		* @remarks <>
		*/
		void dump() const
		{
			DumpT::dump();
		}

	public:
		virtual const char *what() const
		{
			return msg_.c_str();
		}

		std::error_code code() const
		{
			return code_;
		}
	};


#ifdef _DEBUG
	typedef exception_base_t<detail::dump_helper> exception_base;
#else
	typedef exception_base_t<detail::dump_null>	exception_base;
#endif
}




#endif