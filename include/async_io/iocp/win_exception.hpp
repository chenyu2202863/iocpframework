#ifndef __IOCP_WIN_EXCEPTION_HPP
#define __IOCP_WIN_EXCEPTION_HPP


#include "../basic.hpp"
#include "../../exception/exception_base.hpp"



namespace async
{


	namespace iocp
	{
		//-------------------------------------------------------------------
		// class win32_exception

		class win32_exception 
			: public exception::exception_base
		{
		public:
			win32_exception(const std::string &apiName, unsigned long dwErrCode = ::GetLastError());
			virtual ~win32_exception();
		};
	}

}

#endif