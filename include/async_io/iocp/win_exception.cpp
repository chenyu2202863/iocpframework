#include "win_exception.hpp"

#include <sstream>


namespace async
{


	namespace iocp
	{

		win32_exception::win32_exception(const std::string &apiName, unsigned long dwErrCode)
		{
			std::ostringstream oss;
			char *buffer = NULL;
			::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				0, dwErrCode, 0, (LPSTR)&buffer, 0, 0);
			oss << "Win32 Error(" << dwErrCode << ") at " << apiName << ": " << buffer;

			::LocalFree(buffer);
			msg_ += oss.str();
		}

		win32_exception::~win32_exception()
		{
		}
	}


}