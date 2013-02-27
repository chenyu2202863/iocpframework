#ifndef __HTTP_MIME_TYPES_HPP
#define __HTTP_MIME_TYPES_HPP



#include <string>


namespace http
{
	namespace mime_types
	{

		// 把文件的扩展名转换成MIME格式
		std::string ExtensionToType(const std::string &exten);
	}
}






#endif