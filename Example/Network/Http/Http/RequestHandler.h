#ifndef __HTTP_REQUEST_HANDLER_HPP
#define __HTTP_REQUEST_HANDLER_HPP


#include <string>


namespace http
{

	struct Reply;
	struct Request;

	// 所有进入的Request Handler
	class RequestHandler
	{
	private:
		// 文件目录
		std::string docRoot_;

	public:
		// 文件目录名称
		explicit RequestHandler(const std::string& docRoot);

		// 处理请求
		void HandleRequest(const Request& req, Reply& rep);

	
	private:
		static bool _UrlDecode(const std::string& in, std::string& out);
	};
}





#endif