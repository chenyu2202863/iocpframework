#include "stdafx.h"
#include "Reply.h"

#include <sstream>


namespace http
{
	namespace status_strings
	{
		const std::string ok =
			"HTTP/1.0 200 OK\r\n";
		const std::string created =
			"HTTP/1.0 201 Created\r\n";
		const std::string accepted =
			"HTTP/1.0 202 Accepted\r\n";
		const std::string no_content =
			"HTTP/1.0 204 No Content\r\n";
		const std::string multiple_choices =
			"HTTP/1.0 300 Multiple Choices\r\n";
		const std::string moved_permanently =
			"HTTP/1.0 301 Moved Permanently\r\n";
		const std::string moved_temporarily =
			"HTTP/1.0 302 Moved Temporarily\r\n";
		const std::string not_modified =
			"HTTP/1.0 304 Not Modified\r\n";
		const std::string bad_request =
			"HTTP/1.0 400 Bad Request\r\n";
		const std::string unauthorized =
			"HTTP/1.0 401 Unauthorized\r\n";
		const std::string forbidden =
			"HTTP/1.0 403 Forbidden\r\n";
		const std::string not_found =
			"HTTP/1.0 404 Not Found\r\n";
		const std::string internal_server_error =
			"HTTP/1.0 500 Internal Server Error\r\n";
		const std::string not_implemented =
			"HTTP/1.0 501 Not Implemented\r\n";
		const std::string bad_gateway =
			"HTTP/1.0 502 Bad Gateway\r\n";
		const std::string service_unavailable =
			"HTTP/1.0 503 Service Unavailable\r\n";



		const std::string &ToBuffer(Reply::StatusType status)
		{
			switch(status)
			{
			case Reply::ok:
				return ok;
			case Reply::created:
				return created;
			case Reply::accepted:
				return accepted;
			case Reply::no_content:
				return no_content;
			case Reply::multiple_choices:
				return multiple_choices;
			case Reply::moved_permanently:
				return moved_permanently;
			case Reply::moved_temporarily:
				return moved_temporarily;
			case Reply::not_modified:
				return not_modified;
			case Reply::bad_request:
				return bad_request;
			case Reply::unauthorized:
				return unauthorized;
			case Reply::forbidden:
				return forbidden;
			case Reply::not_found:
				return not_found;
			case Reply::internal_server_error:
				return internal_server_error;
			case Reply::not_implemented:
				return not_implemented;
			case Reply::bad_gateway:
				return bad_gateway;
			case Reply::service_unavailable:
				return service_unavailable;
			default:
				return internal_server_error;
			}
		}
	}



	namespace misc_strings
	{
		const char nameValueSeparator[] = { ':', ' ' };
		const char crlf[] = { '\r', '\n' };
	}



	std::vector<async::iocp::ConstBuffer> Reply::ToBuffers()
	{
		std::vector<async::iocp::ConstBuffer> buffers;
		buffers.push_back(async::iocp::Buffer(status_strings::ToBuffer(status)));

		for(std::size_t i = 0; i != headers.size(); ++i)
		{
			Header& h = headers[i];

			buffers.push_back(async::iocp::Buffer(h.name));
			buffers.push_back(async::iocp::Buffer(misc_strings::nameValueSeparator));
			buffers.push_back(async::iocp::Buffer(h.value));
			buffers.push_back(async::iocp::Buffer(misc_strings::crlf));
		}

		buffers.push_back(async::iocp::Buffer(misc_strings::crlf));
		buffers.push_back(async::iocp::Buffer(content));

		return buffers;
	}


	namespace stock_reply
	{


		const char ok[] = "";
		const char created[] =
			"<html>"
			"<head><title>Created</title></head>"
			"<body><h1>201 Created</h1></body>"
			"</html>";
		const char accepted[] =
			"<html>"
			"<head><title>Accepted</title></head>"
			"<body><h1>202 Accepted</h1></body>"
			"</html>";
		const char no_content[] =
			"<html>"
			"<head><title>No Content</title></head>"
			"<body><h1>204 Content</h1></body>"
			"</html>";
		const char multiple_choices[] =
			"<html>"
			"<head><title>Multiple Choices</title></head>"
			"<body><h1>300 Multiple Choices</h1></body>"
			"</html>";
		const char moved_permanently[] =
			"<html>"
			"<head><title>Moved Permanently</title></head>"
			"<body><h1>301 Moved Permanently</h1></body>"
			"</html>";
		const char moved_temporarily[] =
			"<html>"
			"<head><title>Moved Temporarily</title></head>"
			"<body><h1>302 Moved Temporarily</h1></body>"
			"</html>";
		const char not_modified[] =
			"<html>"
			"<head><title>Not Modified</title></head>"
			"<body><h1>304 Not Modified</h1></body>"
			"</html>";
		const char bad_request[] =
			"<html>"
			"<head><title>Bad Request</title></head>"
			"<body><h1>400 Bad Request</h1></body>"
			"</html>";
		const char unauthorized[] =
			"<html>"
			"<head><title>Unauthorized</title></head>"
			"<body><h1>401 Unauthorized</h1></body>"
			"</html>";
		const char forbidden[] =
			"<html>"
			"<head><title>Forbidden</title></head>"
			"<body><h1>403 Forbidden</h1></body>"
			"</html>";
		const char not_found[] =
			"<html>"
			"<head><title>Not Found</title></head>"
			"<body><h1>404 Not Found</h1></body>"
			"</html>";
		const char internal_server_error[] =
			"<html>"
			"<head><title>Internal Server Error</title></head>"
			"<body><h1>500 Internal Server Error</h1></body>"
			"</html>";
		const char not_implemented[] =
			"<html>"
			"<head><title>Not Implemented</title></head>"
			"<body><h1>501 Not Implemented</h1></body>"
			"</html>";
		const char bad_gateway[] =
			"<html>"
			"<head><title>Bad Gateway</title></head>"
			"<body><h1>502 Bad Gateway</h1></body>"
			"</html>";
		const char service_unavailable[] =
			"<html>"
			"<head><title>Service Unavailable</title></head>"
			"<body><h1>503 Service Unavailable</h1></body>"
			"</html>";


		std::string ToString(Reply::StatusType status)
		{
			switch (status)
			{
			case Reply::ok:
				return ok;
			case Reply::created:
				return created;
			case Reply::accepted:
				return accepted;
			case Reply::no_content:
				return no_content;
			case Reply::multiple_choices:
				return multiple_choices;
			case Reply::moved_permanently:
				return moved_permanently;
			case Reply::moved_temporarily:
				return moved_temporarily;
			case Reply::not_modified:
				return not_modified;
			case Reply::bad_request:
				return bad_request;
			case Reply::unauthorized:
				return unauthorized;
			case Reply::forbidden:
				return forbidden;
			case Reply::not_found:
				return not_found;
			case Reply::internal_server_error:
				return internal_server_error;
			case Reply::not_implemented:
				return not_implemented;
			case Reply::bad_gateway:
				return bad_gateway;
			case Reply::service_unavailable:
				return service_unavailable;
			default:
				return internal_server_error;
			}
		}
	}




	Reply Reply::StockReply(Reply::StatusType status)
	{
		Reply rep;

		rep.status	= status;
		rep.content = stock_reply::ToString(status);
		rep.headers.resize(2);

		rep.headers[0].name		= "Content-Length";

		std::stringstream stream;
		stream << rep.content.size();

		rep.headers[0].value	= stream.str();
		rep.headers[1].name		= "Content-Type";
		rep.headers[1].value	= "text/html";

		return rep;
	}
}