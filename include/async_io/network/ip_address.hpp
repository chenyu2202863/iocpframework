#ifndef __NETWORK_IP_ADDRESS_HPP
#define __NETWORK_IP_ADDRESS_HPP


#include "../basic.hpp"
#include <string>


namespace async
{


	namespace network
	{

		class ip_address 
		{
		private:
			u_long address_;

		public:
			ip_address(u_long address);

		public:
			u_long address() const
			{ return address_; }
			operator u_long() const
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