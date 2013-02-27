#include "socket_provider.hpp"
#include "../IOCP/win_exception.hpp"
#include "socket_option.hpp"





namespace async
{
	namespace network
	{

		socket_provider sck_provider;

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
			get_extension_function(sck, guidWSARecvMsg,			&WSARecvMsg);
		}

		socket_provider::~socket_provider()
		{
		}

		socket_provider &socket_provider::singleton()
		{
			return sck_provider;
		}

		void socket_provider::get_extension_function(SOCKET &sock, const GUID &guid, LPVOID pFunc)
		{
			extension_function ext_func(guid, pFunc);

			DWORD dwRet = 0;
			if( 0 != ::WSAIoctl(sock, ext_func.cmd(), ext_func.in_buffer(), ext_func.in_buffer_size(), 
				ext_func.out_buffer(), ext_func.out_buffer_size(), &dwRet, 0, 0) )
				throw iocp::win32_exception("WSAIoCtl");
		}

		void socket_provider::cancel_io(SOCKET socket)
		{
			if( FARPROC cancelFuncPtr = ::GetProcAddress(::GetModuleHandleA("KERNEL32"), "CancelIoEx") )
			{
				// 仅在Vista以后支持，可以从不同的线程来取消IO操作
				typedef BOOL (__stdcall *CancelIOExPtr)(HANDLE, LPOVERLAPPED);
				CancelIOExPtr cancelIOEx = reinterpret_cast<CancelIOExPtr>(cancelFuncPtr);

				if( !cancelIOEx(reinterpret_cast<HANDLE>(socket), 0) )
					throw iocp::win32_exception("CancelIOEx");
			}
			else
			{
				// 此处忽略了在同一线程的检查
				// CancelIO只能在同一个线程中取消IO操作
				if( !::CancelIo(reinterpret_cast<HANDLE>(socket)) )
					throw iocp::win32_exception("CancelIo");
			}
		}
	}

}