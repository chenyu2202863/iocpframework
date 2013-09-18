#include "socket_provider.hpp"

#include <mutex>

#include "../service/exception.hpp"
#include "socket_option.hpp"





namespace async { namespace network {

	socket_provider::socket_provider()
	{
		static GUID guidTransmitFile		= WSAID_TRANSMITFILE;
		static GUID guidAcceptEx			= WSAID_ACCEPTEX;
		static GUID guidGetAcceptExSockaddrs= WSAID_GETACCEPTEXSOCKADDRS;
		static GUID guidTransmitPackets		= WSAID_TRANSMITPACKETS;
		static GUID guidConnectEx			= WSAID_CONNECTEX;
		static GUID guidDisconnectEx		= WSAID_DISCONNECTEX;
		static GUID guidWSARecvMsg			= WSAID_WSARECVMSG;

		SOCKET sck = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		std::shared_ptr<void> val = std::shared_ptr<void>(nullptr, [&sck](void *tmp){::closesocket(sck);});

		get_extension_function(sck, guidTransmitFile,			&TransmitFile);
		get_extension_function(sck, guidAcceptEx,				&AcceptEx);
		get_extension_function(sck, guidGetAcceptExSockaddrs,	&GetAcceptExSockaddrs);
		get_extension_function(sck, guidTransmitPackets,		&TransmitPackets);
		get_extension_function(sck, guidConnectEx,				&ConnectEx);
		get_extension_function(sck, guidDisconnectEx,			&DisconnectEx);
		get_extension_function(sck, guidWSARecvMsg,				&WSARecvMsg);
	}

	socket_provider::~socket_provider()
	{
	}

	socket_provider &socket_provider::singleton()
	{
		static std::once_flag flag;
		static std::shared_ptr<socket_provider> provider;

		std::call_once(flag, [](){ provider.reset(new socket_provider); });

		return *provider;
	}

	void socket_provider::get_extension_function(SOCKET &sock, const GUID &guid, LPVOID pFunc)
	{
		extension_function ext_func(guid, pFunc);

		DWORD dwRet = 0;
		if( 0 != ::WSAIoctl(sock, ext_func.cmd(), ext_func.in_buffer(), ext_func.in_buffer_size(), 
			ext_func.out_buffer(), ext_func.out_buffer_size(), &dwRet, 0, 0) )
			throw service::win32_exception_t("WSAIoCtl");
	}

	void socket_provider::cancel_io(SOCKET socket)
	{
		::CancelIoEx(reinterpret_cast<HANDLE>(socket), nullptr);
	}
}

}