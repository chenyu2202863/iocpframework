#include "ip_address.hpp"

#include "../basic.hpp"

namespace async { namespace network {

	ip_address::ip_address(std::uint32_t address)
		: address_(address)
	{
	}


	ip_address ip_address::parse(const std::string &str)
	{
		std::uint32_t address = ::ntohl(::inet_addr(str.c_str()));

		return ip_address(address);
	}

	std::string	ip_address::parse(const ip_address &addr)
	{
		in_addr tmp = {0};
		tmp.s_addr = addr.address();

		char *p = ::inet_ntoa(tmp);
		return p;
	}
}

}