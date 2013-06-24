#ifndef __ASYNC_NETWORK_IP_ADDRESS_HPP
#define __ASYNC_NETWORK_IP_ADDRESS_HPP

#include <cstdint>
#include <string>


namespace async { namespace network {

		class ip_address 
		{
		private:
			std::uint32_t address_;

		public:
			ip_address(std::uint32_t address);

		public:
			std::uint32_t address() const
			{ return address_; }

			bool operator==(const ip_address &ipaddr)
			{ return address_ == ipaddr.address_; }

			bool operator!=(const ip_address &ipaddr)
			{ return address_ != ipaddr.address_; }

		public:
			static ip_address parse(const std::string &str);
			static std::string parse(const ip_address &addr);
		};
	}


}



#endif