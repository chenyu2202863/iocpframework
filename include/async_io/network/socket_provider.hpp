#ifndef __ASYNC_NETWORK_PROVIDER_HPP
#define __ASYNC_NETWORK_PROVIDER_HPP

#include "../basic.hpp"
#include "sock_init.hpp"



namespace async { namespace network {

		class socket_provider
		{
			details::sock_init_t<> sockInit_;	

		public:
			explicit socket_provider();
			~socket_provider();

			// non-copyable
		private:
			socket_provider(const socket_provider &);
			socket_provider &operator=(const socket_provider &);

		public:
			LPFN_TRANSMITFILE			TransmitFile;
			LPFN_ACCEPTEX				AcceptEx;
			LPFN_GETACCEPTEXSOCKADDRS	GetAcceptExSockaddrs;
			LPFN_TRANSMITPACKETS		TransmitPackets;
			LPFN_CONNECTEX				ConnectEx;
			LPFN_DISCONNECTEX			DisconnectEx;
			LPFN_WSARECVMSG				WSARecvMsg;
			

		public:
			// 提供唯一实例
			static socket_provider &singleton();

			// 获取扩展API
			static void get_extension_function(SOCKET &sock, const GUID &guid, LPVOID pFunc);

			static void cancel_io(SOCKET sock);
		};
	}

}



#endif