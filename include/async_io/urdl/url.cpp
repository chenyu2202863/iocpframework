#include "url.hpp"
#include "http.hpp"

#include <cstring>
#include <cctype>
#include <cstdlib>
#include <exception>



namespace urdl 
{

	unsigned short url::port() const
	{
		if (!port_.empty())
			return std::atoi(port_.c_str());
		if (protocol_ == "http")
			return 80;
		if (protocol_ == "https")
			return 443;
		if (protocol_ == "ftp")
			return 21;
		return 0;
	}

	std::string url::path() const
	{
		std::string tmp_path;
		_unescape_path(path_, tmp_path);
		return tmp_path;
	}

	std::string url::to_string(int components) const
	{
		std::string s;

		if ((components & protocol_component) != 0 && !protocol_.empty())
		{
			s = protocol_;
			s += "://";
		}

		if ((components & user_info_component) != 0 && !user_info_.empty())
		{
			s += user_info_;
			s += "@";
		}

		if ((components & host_component) != 0)
		{
			s += host_;
		}

		if ((components & port_component) != 0 && !port_.empty())
		{
			s += ":";
			s += port_;
		}

		if ((components & path_component) != 0 && !path_.empty())
		{
			s += path_;
		}

		if ((components & query_component) != 0 && !query_.empty())
		{
			s += "?";
			s += query_;
		}

		if ((components & fragment_component) != 0 && !fragment_.empty())
		{
			s += "#";
			s += fragment_;
		}

		return s;
	}

	url url::from_string(const char* s, std::error_code& ec)
	{
		url new_url;

		// Protocol.
		std::size_t length = std::strcspn(s, ":");
		new_url.protocol_.assign(s, s + length);
		for (std::size_t i = 0; i < new_url.protocol_.length(); ++i)
			new_url.protocol_[i] = std::tolower(new_url.protocol_[i]);
		s += length;

		// "://".
		if (*s++ != ':')
		{
			ec = std::make_error_code(std::errc::invalid_argument);
			return url();
		}
		if (*s++ != '/')
		{
			ec = std::make_error_code(std::errc::invalid_argument);
			return url();
		}
		if (*s++ != '/')
		{
			ec = std::make_error_code(std::errc::invalid_argument);
			return url();
		}

		// UserInfo.
		length = std::strcspn(s, "@:[/?#");
		if (s[length] == '@')
		{
			new_url.user_info_.assign(s, s + length);
			s += length + 1;
		}
		else if (s[length] == ':')
		{
			std::size_t length2 = std::strcspn(s + length, "@/?#");
			if (s[length + length2] == '@')
			{
				new_url.user_info_.assign(s, s + length + length2);
				s += length + length2 + 1;
			}
		}

		// Host.
		if (*s == '[')
		{
			length = std::strcspn(++s, "]");
			if (s[length] != ']')
			{
				ec = std::make_error_code(std::errc::invalid_argument);
				return url();
			}
			new_url.host_.assign(s, s + length);
			s += length + 1;
			if (std::strcspn(s, ":/?#") != 0)
			{
				ec = std::make_error_code(std::errc::invalid_argument);
				return url();
			}
		}
		else
		{
			length = std::strcspn(s, ":/?#");
			new_url.host_.assign(s, s + length);
			s += length;
		}

		// Port.
		if (*s == ':')
		{
			length = std::strcspn(++s, "/?#");
			if (length == 0)
			{
				ec = std::make_error_code(std::errc::invalid_argument);
				return url();
			}
			new_url.port_.assign(s, s + length);
			for (std::size_t i = 0; i < new_url.port_.length(); ++i)
			{
				if (!std::isdigit(new_url.port_[i]))
				{
					ec = std::make_error_code(std::errc::invalid_argument);
					return url();
				}
			}
			s += length;
		}

		// Path.
		if (*s == '/')
		{
			length = std::strcspn(s, "?#");
			new_url.path_.assign(s, s + length);
			std::string tmp_path;
			if (!_unescape_path(new_url.path_, tmp_path))
			{
				ec = std::make_error_code(std::errc::invalid_argument);
				return url();
			}
			s += length;
		}
		else
			new_url.path_ = "/";

		// Query.
		if (*s == '?')
		{
			length = std::strcspn(++s, "#");
			new_url.query_.assign(s, s + length);
			s += length;
		}

		// Fragment.
		if (*s == '#')
			new_url.fragment_.assign(++s);

		ec = std::error_code();
		return new_url;
	}

	url url::from_string(const char* s)
	{
		std::error_code ec;
		url new_url(from_string(s, ec));
		if (ec)
		{
			throw std::system_error(ec);
		}
		return new_url;
	}

	url url::from_string(const std::string& s, std::error_code& ec)
	{
		return from_string(s.c_str(), ec);
	}

	url url::from_string(const std::string& s)
	{
		return from_string(s.c_str());
	}

	bool url::_unescape_path(const std::string& in, std::string& out)
	{
		out.clear();
		out.reserve(in.size());

		for(std::size_t i = 0; i < in.size(); ++i)
		{
			switch (in[i])
			{
			case '%':
				if (i + 3 <= in.size())
				{
					unsigned int value = 0;
					for (std::size_t j = i + 1; j < i + 3; ++j)
					{
						switch (in[j])
						{
						case '0': case '1': case '2': case '3': case '4':
						case '5': case '6': case '7': case '8': case '9':
							value += in[j] - '0';
							break;
						case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
							value += in[j] - 'a' + 10;
							break;
						case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
							value += in[j] - 'A' + 10;
							break;
						default:
							return false;
						}
						if (j == i + 1)
							value <<= 4;
					}
					out += static_cast<char>(value);
					i += 2;
				}
				else
					return false;
				break;
			case '-': case '_': case '.': case '!': case '~': case '*':
			case '\'': case '(': case ')': case ':': case '@': case '&':
			case '=': case '+': case '$': case ',': case '/': case ';':
				out += in[i];
				break;
			default:
				if (!std::isalnum(in[i]))
					return false;
				out += in[i];
				break;
			}
		}
		return true;
	}

	bool operator==(const url& a, const url& b)
	{
		return a.protocol_ == b.protocol_
			&& a.user_info_ == b.user_info_
			&& a.host_ == b.host_
			&& a.port_ == b.port_
			&& a.path_ == b.path_
			&& a.query_ == b.query_
			&& a.fragment_ == b.fragment_;
	}

	bool operator!=(const url& a, const url& b)
	{
		return !(a == b);
	}

	bool operator<(const url& a, const url& b)
	{
		if (a.protocol_ < b.protocol_)
			return true;
		if (b.protocol_ < a.protocol_)
			return false;

		if (a.user_info_ < b.user_info_)
			return true;
		if (b.user_info_ < a.user_info_)
			return false;

		if (a.host_ < b.host_)
			return true;
		if (b.host_ < a.host_)
			return false;

		if (a.port_ < b.port_)
			return true;
		if (b.port_ < a.port_)
			return false;

		if (a.path_ < b.path_)
			return true;
		if (b.path_ < a.path_)
			return false;

		if (a.query_ < b.query_)
			return true;
		if (b.query_ < a.query_)
			return false;

		return a.fragment_ < b.fragment_;
	}

} // namespace urdl



