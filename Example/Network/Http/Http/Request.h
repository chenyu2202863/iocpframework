#ifndef __HTTP_REQUEST_HPP
#define __HTTP_REQUEST_HPP



#include <string>
#include <vector>
#include "Header.h"



namespace http
{

	// 从客服端接收到的请求

	struct Request
	{
		std::string method;
		std::string uri;
		int httpVersionMajor;
		int httpVersionMinor;

		std::vector<Header> headers;
	};
}




#endif