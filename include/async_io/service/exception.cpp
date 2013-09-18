#include "exception.hpp"

#include <sstream>


namespace async { namespace service {

	win32_exception_t::win32_exception_t(const std::string &msg, std::uint32_t error)
		: ::exception::exception_base(std::make_error_code((std::errc)error), "")
	{
		std::ostringstream oss;
		char *buffer = NULL;
		::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			0, error, 0, (LPSTR)&buffer, 0, 0);
		oss << "Win32 Error(" << error << ") at " << msg << ": " << buffer;

		::LocalFree(buffer);
		msg_ = oss.str();
	}

	win32_exception_t::~win32_exception_t()
	{
	}

	
	network_exception::network_exception(const std::string &msg)
		: ::exception::exception_base(std::make_error_code(std::errc::io_error), msg)
	{

	}
}


}