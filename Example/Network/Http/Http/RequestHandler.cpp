#include "stdafx.h"
#include "RequestHandler.h"


#include <fstream>
#include <sstream>
#include <string>

#include "Mimetypes.h"
#include "Reply.h"
#include "Request.h"


namespace http
{
	
	RequestHandler::RequestHandler(const std::string& docRoot)
		: docRoot_(docRoot)
	{
	}

	void RequestHandler::HandleRequest(const Request& req, Reply& rep)
	{
		// Decode url to path.
		std::string request_path;
		if( !_UrlDecode(req.uri, request_path) )
		{
			rep = Reply::StockReply(Reply::bad_request);
			return;
		}

		// 请求路径必须为绝对路径，不能包含相对路径的..
		if( request_path.empty() || request_path[0] != '/'
			|| request_path.find("..") != std::string::npos )
		{
			rep = Reply::StockReply(Reply::bad_request);
			return;
		}

		// 如果路径的最后包含'/',则在最后需要加上"index.html"
		if( request_path[request_path.size() - 1] == '/' )
		{
			request_path += "index.html";
		}

		// Determine the file extension.
		std::size_t last_slash_pos = request_path.find_last_of("/");
		std::size_t last_dot_pos = request_path.find_last_of(".");
		std::string extension;
		if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
		{
			extension = request_path.substr(last_dot_pos + 1);
		}

		// Open the file to send back.
		std::string full_path = docRoot_ + request_path;

		// 读取文件
		std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
		if (!is)
		{
			rep = Reply::StockReply(Reply::not_found);
			return;
		}

		// Fill out the reply to be sent to the client.
		rep.status = Reply::ok;
		char buf[512] = {0};

		while(is.read(buf, sizeof(buf) / sizeof(buf[0])).gcount() > 0)
			rep.content.append(buf, is.gcount());

		rep.headers.resize(2);
		rep.headers[0].name		= "Content-Length";

		std::stringstream stream;
		stream << rep.content.size();

		rep.headers[0].value	= stream.str();
		rep.headers[1].name		= "Content-Type";
		rep.headers[1].value	= mime_types::ExtensionToType(extension);
	}

	bool RequestHandler::_UrlDecode(const std::string& in, std::string& out)
	{
		out.clear();
		out.reserve(in.size());

		for(std::size_t i = 0; i < in.size(); ++i)
		{
			if (in[i] == '%')
			{
				if (i + 3 <= in.size())
				{
					int value = 0;
					std::istringstream is(in.substr(i + 1, 2));
					if (is >> std::hex >> value)
					{
						out += static_cast<char>(value);
						i += 2;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else if (in[i] == '+')
			{
				out += ' ';
			}
			else
			{
				out += in[i];
			}
		}

		return true;
	}
}