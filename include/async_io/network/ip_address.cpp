#include "ip_address.hpp"


namespace async
{

	namespace network
	{

		ip_address::ip_address(u_long address)
			: address_(address)
		{
		}


		ip_address ip_address::parse(const std::string &str)
		{
			u_long address = ::ntohl(::inet_addr(str.c_str()));

			return ip_address(address);
		}

		std::string	ip_address::parse(const ip_address &addr)
		{
			in_addr tmp = {0};
			tmp.s_addr = addr;

            char *p = ::inet_ntoa(tmp);
			return p;
		}
	}

}