#include "stdafx.h"
#include "RequestParser.h"


#include "Request.h"



namespace http
{
	RequestParser::RequestParser()
		: state_(method_start)
	{
	}

	void RequestParser::Reset()
	{
		state_ = method_start;
	}

	ParseRet RequestParser::Consume(Request& req, char input)
	{
		switch (state_)
		{
		case method_start:
			if (!isChar(input) || isCtl(input) || isTSpecial(input))
			{
				return FALSE_VALUE;
			}
			else
			{
				state_ = method;
				req.method.push_back(input);
				return INDETERMINATE;
			}
		case method:
			if (input == ' ')
			{
				state_ = uri;
				return INDETERMINATE;
			}
			else if (!isChar(input) || isCtl(input) || isTSpecial(input))
			{
				return FALSE_VALUE;
			}
			else
			{
				req.method.push_back(input);
				return INDETERMINATE;
			}
		case uri_start:
			if (isCtl(input))
			{
				return FALSE_VALUE;
			}
			else
			{
				state_ = uri;
				req.uri.push_back(input);
				return INDETERMINATE;
			}
		case uri:
			if (input == ' ')
			{
				state_ = http_version_h;
				return INDETERMINATE;
			}
			else if (isCtl(input))
			{
				return FALSE_VALUE;
			}
			else
			{
				req.uri.push_back(input);
				return INDETERMINATE;
			}
		case http_version_h:
			if (input == 'H')
			{
				state_ = http_version_t_1;
				return INDETERMINATE;
			}
			else
			{
				return FALSE_VALUE;
			}
		case http_version_t_1:
			if (input == 'T')
			{
				state_ = http_version_t_2;
				return INDETERMINATE;
			}
			else
			{
				return FALSE_VALUE;
			}
		case http_version_t_2:
			if (input == 'T')
			{
				state_ = http_version_p;
				return INDETERMINATE;
			}
			else
			{
				return FALSE_VALUE;
			}
		case http_version_p:
			if (input == 'P')
			{
				state_ = http_version_slash;
				return INDETERMINATE;
			}
			else
			{
				return FALSE_VALUE;
			}
		case http_version_slash:
			if (input == '/')
			{
				req.httpVersionMajor = 0;
				req.httpVersionMinor = 0;
				state_ = http_version_major_start;
				return INDETERMINATE;
			}
			else
			{
				return FALSE_VALUE;
			}
		case http_version_major_start:
			if (isDigit(input))
			{
				req.httpVersionMajor = req.httpVersionMajor * 10 + input - '0';
				state_ = http_version_major;
				return INDETERMINATE;
			}
			else
			{
				return FALSE_VALUE;
			}
		case http_version_major:
			if (input == '.')
			{
				state_ = http_version_minor_start;
				return INDETERMINATE;
			}
			else if (isDigit(input))
			{
				req.httpVersionMajor = req.httpVersionMajor * 10 + input - '0';
				return INDETERMINATE;
			}
			else
			{
				return FALSE_VALUE;
			}
		case http_version_minor_start:
			if (isDigit(input))
			{
				req.httpVersionMinor = req.httpVersionMinor * 10 + input - '0';
				state_ = http_version_minor;
				return INDETERMINATE;
			}
			else
			{
				return FALSE_VALUE;
			}
		case http_version_minor:
			if (input == '\r')
			{
				state_ = expecting_newline_1;
				return INDETERMINATE;
			}
			else if (isDigit(input))
			{
				req.httpVersionMinor = req.httpVersionMinor * 10 + input - '0';
				return INDETERMINATE;
			}
			else
			{
				return FALSE_VALUE;
			}
		case expecting_newline_1:
			if (input == '\n')
			{
				state_ = header_line_start;
				return INDETERMINATE;
			}
			else
			{
				return FALSE_VALUE;
			}
		case header_line_start:
			if (input == '\r')
			{
				state_ = expecting_newline_3;
				return INDETERMINATE;
			}
			else if (!req.headers.empty() && (input == ' ' || input == '\t'))
			{
				state_ = header_lws;
				return INDETERMINATE;
			}
			else if (!isChar(input) || isCtl(input) || isTSpecial(input))
			{
				return FALSE_VALUE;
			}
			else
			{
				req.headers.push_back(Header());
				req.headers.back().name.push_back(input);
				state_ = header_name;
				return INDETERMINATE;
			}
		case header_lws:
			if (input == '\r')
			{
				state_ = expecting_newline_2;
				return INDETERMINATE;
			}
			else if (input == ' ' || input == '\t')
			{
				return INDETERMINATE;
			}
			else if (isCtl(input))
			{
				return FALSE_VALUE;
			}
			else
			{
				state_ = header_value;
				req.headers.back().value.push_back(input);
				return INDETERMINATE;
			}
		case header_name:
			if (input == ':')
			{
				state_ = space_before_header_value;
				return INDETERMINATE;
			}
			else if (!isChar(input) || isCtl(input) || isTSpecial(input))
			{
				return FALSE_VALUE;
			}
			else
			{
				req.headers.back().name.push_back(input);
				return INDETERMINATE;
			}
		case space_before_header_value:
			if (input == ' ')
			{
				state_ = header_value;
				return INDETERMINATE;
			}
			else
			{
				return FALSE_VALUE;
			}
		case header_value:
			if (input == '\r')
			{
				state_ = expecting_newline_2;
				return INDETERMINATE;
			}
			else if (isCtl(input))
			{
				return FALSE_VALUE;
			}
			else
			{
				req.headers.back().value.push_back(input);
				return INDETERMINATE;
			}
		case expecting_newline_2:
			if (input == '\n')
			{
				state_ = header_line_start;
				return INDETERMINATE;
			}
			else
			{
				return FALSE_VALUE;
			}
		case expecting_newline_3:
			return (input == '\n') ? TRUE_VALUE : FALSE_VALUE;
		default:
			return FALSE_VALUE;
		}
	}

	bool RequestParser::isChar(int c)
	{
		return c >= 0 && c <= 127;
	}

	bool RequestParser::isCtl(int c)
	{
		return (c >= 0 && c <= 31) || (c == 127);
	}

	bool RequestParser::isTSpecial(int c)
	{
		switch (c)
		{
		case '(': case ')': case '<': case '>': case '@':
		case ',': case ';': case ':': case '\\': case '"':
		case '/': case '[': case ']': case '?': case '=':
		case '{': case '}': case ' ': case '\t':
			return true;
		default:
			return false;
		}
	}

	bool RequestParser::isDigit(int c)
	{
		return c >= '0' && c <= '9';
	}
}