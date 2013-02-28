#ifndef __HTTP_REQUEST_PARSER_HPP
#define __HTTP_REQUEST_PARSER_HPP

#include <tuple>




namespace http
{

	// 请求解析后返回值
	enum ParseRet
	{
		TRUE_VALUE,
		FALSE_VALUE,
		INDETERMINATE
	};


	struct Request;

	// 解析请求
	class RequestParser
	{
		// The current state of the parser.
		enum state
		{
			method_start,
			method,
			uri_start,
			uri,
			http_version_h,
			http_version_t_1,
			http_version_t_2,
			http_version_p,
			http_version_slash,
			http_version_major_start,
			http_version_major,
			http_version_minor_start,
			http_version_minor,
			expecting_newline_1,
			header_line_start,
			header_lws,
			header_name,
			space_before_header_value,
			header_value,
			expecting_newline_2,
			expecting_newline_3
		} state_;

	public:
		RequestParser();

	public:
		//重置状态
		void Reset();

		// 返回值 TRUE_VALUE, FALSE_VALUE, INDETERMINATE
		template<typename InputIteratorT>
		std::tuple<ParseRet, ParseRet> Parse(Request& req, InputIteratorT begin, InputIteratorT end)
		{
			while(begin != end)
			{
				ParseRet result = Consume(req, *begin++);
				if( result != INDETERMINATE )
					return std::make_tuple(result, result);
			}
			
			return std::make_tuple(INDETERMINATE, INDETERMINATE);
		}

	private:
		// 处理下一个输入字符
		ParseRet Consume(Request& req, char input);

		// 检测是否为HTTP字符
		static bool isChar(int c);

		//检测是否为HTTP控制码
		static bool isCtl(int c);

		//检测是否为HTTP特殊码
		static bool isTSpecial(int c);

		// 检测是否为数字
		static bool isDigit(int c);
	};
		
}






#endif