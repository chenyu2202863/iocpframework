#ifndef __IOCP_WIN_EXCEPTION_HPP
#define __IOCP_WIN_EXCEPTION_HPP

#include <cstdint>
#include <string>

#include "../../exception/exception_base.hpp"


namespace async { namespace service {
		
	//-------------------------------------------------------------------
		// class win32_exception

		class win32_exception_t 
			: public exception::exception_base
		{
		public:
			win32_exception_t(const std::string &msg, std::uint32_t error = ::GetLastError());
			virtual ~win32_exception_t();
		};

		// ------------------------------------------------------------------
		// class network_exception
		
		class network_exception
			: public exception::exception_base
		{
		public:
			network_exception(const std::string &msg);
		};
	}

}

#endif