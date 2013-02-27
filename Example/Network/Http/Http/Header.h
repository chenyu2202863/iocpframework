#ifndef __HTTP_HEADER_HPP
#define __HTTP_HEADER_HPP


#include <string>


namespace http
{

	struct Header
	{
		std::string name;
		std::string value;
	};

}


#endif