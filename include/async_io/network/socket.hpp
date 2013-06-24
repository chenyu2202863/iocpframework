#ifndef __ASYNC_NETWORK_SOCKET_HPP
#define __ASYNC_NETWORK_SOCKET_HPP

#include "../service/dispatcher.hpp"
#include "../service/read_write_buffer.hpp"
#include "../service/multi_buffer.hpp"

#include "ip_address.hpp"
#include "socket_provider.hpp"


namespace async { namespace network {
	// forward declare

	class socket_handle_t;
	typedef std::unique_ptr<socket_handle_t> socket_handle_ptr;

	//----------------------------------------------------------------------
	// class Socket

	class socket_handle_t
	{
	public:
		typedef service::io_dispatcher_t	dispatcher_type;
		typedef SOCKET native_handle_type;

	private:
		// socket handle
		native_handle_type socket_;

		// IO服务
		dispatcher_type &io_;

	public:
		explicit socket_handle_t(dispatcher_type &);
		socket_handle_t(dispatcher_type &, native_handle_type sock);
		socket_handle_t(dispatcher_type &, int family, int type, int protocol);

		socket_handle_t(socket_handle_t &&);
		socket_handle_t &operator=(socket_handle_t &&);

		~socket_handle_t();

	private:
		socket_handle_t(const socket_handle_t &);
		socket_handle_t &operator=(const socket_handle_t &);

	public:
		// explicit转换
		operator native_handle_type &()				{ return socket_; }
		operator const native_handle_type &() const	{ return socket_; }

		// 显示获取
		native_handle_type &native_handle()				{ return socket_; }
		const native_handle_type &native_handle() const	{ return socket_; }

	public:
		// WSAIoctl
		template < typename IOCtrlT >
		bool io_control(IOCtrlT &ioCtrl);

		// setsockopt
		template < typename SocketOptionT >
		bool set_option(const SocketOptionT &option);

		// getsockopt
		template < typename SocketOptionT >
		bool get_option(SocketOptionT &option) const;

		// WSASocket
		void open(int family, int nType, int nProtocol);
		// shutdown
		void shutdown(int shut);
		// closesocket
		void close();

		bool is_open() const;
		// CancelIO/CancelIOEx
		void cancel();

		// bind
		void bind(int family, std::uint16_t uPort, const ip_address &addr);
		// listen
		void listen(int nMax);

		dispatcher_type &get_dispatcher()
		{
			return io_;
		}

		// 不需设置回调接口,同步函数
	public:
		socket_handle_ptr accept();
		void connect(int family, const ip_address &addr, std::uint16_t uPort);
		void dis_connect(int shut, bool bReuseSocket = true);

		size_t read(service::mutable_buffer_t &buffer, DWORD flag);
		size_t write(const service::const_buffer_t &buffer, DWORD flag);

		size_t send_to(const service::const_buffer_t &buf, const SOCKADDR_IN *addr, DWORD flag);
		size_t recv_from(service::mutable_buffer_t &buf, SOCKADDR_IN *addr, DWORD flag);

		// 异步调用接口
	public:
		// szOutSize指定额外的缓冲区大小，以用来Accept远程连接后且收到第一块数据包才返回
		template < typename HandlerT >
		void async_accept(socket_handle_t &&remote_sck, HandlerT &&callback);
		// 异步连接需要先绑定端口
		template < typename HandlerT >
		void async_connect(const ip_address &addr, std::uint16_t uPort, HandlerT &&callback);

		// 异步断开连接
		template < typename HandlerT >
		void async_disconnect(bool is_reuse, HandlerT &&callback);

		// 异步TCP读取
		template < typename HandlerT >
		void async_read(service::mutable_buffer_t &buf, HandlerT &&callback);
		
		// 异步TCP写入
		template < typename HandlerT >
		void async_write(const service::const_buffer_t &buf, HandlerT &&callback);
		template < typename HandlerT >
		void async_write(const service::const_array_buffer_t &buf, HandlerT &&callback);
		template < typename ParamT >
		void async_write(ParamT &&param, 
			typename std::enable_if<ParamT::is_static_param_t::value>::type * = nullptr);
		template < typename ParamT >
		void async_write(ParamT &&param, 
			typename std::enable_if<!ParamT::is_static_param_t::value>::type * = nullptr);

		// 异步UDP读取
		template < typename HandlerT >
		void async_send_to(const service::const_buffer_t &buf, const SOCKADDR_IN *addr, HandlerT &&callback);
		// 异步UDP读入
		template < typename HandlerT >
		void async_recv_from(service::mutable_buffer_t &buf, SOCKADDR_IN *addr, HandlerT &&callback);
	
	private:
		void _async_write_impl(const service::const_array_buffer_t &const_array, service::async_callback_base_ptr &ptr);
	};
}
}


#include "accept.hpp"
#include "connect.hpp"

namespace async { namespace network {

	// ---------------------------

	inline bool socket_handle_t::is_open() const
	{
		return socket_ != INVALID_SOCKET;
	}


	template< typename IOCtrlT >
	inline bool socket_handle_t::io_control(IOCtrlT &ioCtrl)
	{
		if( !is_open() )
			return false;

		DWORD dwRet = 0;
		if( 0 != ::WSAIoctl(socket_, ioCtrl.cmd(), ioCtrl.in_buffer(), ioCtrl.in_buffer_size(), 
			ioCtrl.out_buffer(), ioCtrl.out_buffer_size(), &dwRet, 0, 0) )
			throw service::win32_exception_t("WSAIoCtl");

		return true;
	}

	template<typename SocketOptionT>
	inline bool socket_handle_t::set_option(const SocketOptionT &option)
	{
		if( !is_open() )
			return false;

		return SOCKET_ERROR != ::setsockopt(socket_, option.level(), option.name(), option.data(), option.size());
	}

	template<typename SocketOptionT>
	inline bool socket_handle_t::get_option(SocketOptionT &option) const
	{
		if( !is_open() )
			return false;

		int sz = option.size();
		if( SOCKET_ERROR != ::getsockopt(socket_, option.level(), option.name(), option.data(), &sz) )
		{
			option.resize(sz);
			return true;
		}
		else
			return false;
	}

	template < typename HandlerT >
	void socket_handle_t::async_accept(socket_handle_t &&remote_sck, HandlerT &&callback)
	{
		if( !is_open() ) 
			throw service::network_exception("Socket not open");

		native_handle_type sck = remote_sck.native_handle();

		typedef details::accept_handle_t<HandlerT> HookAcceptor;
		HookAcceptor accept_hook(*this, std::move(remote_sck), std::forward<HandlerT>(callback));
		auto p = service::make_async_callback(std::move(accept_hook));
		service::async_callback_base_ptr async_result(p);

		// 根据szOutSide大小判断，是否需要接收远程客户机第一块数据才返回。
		// 如果为0，则立即返回。若大于0，则接受数据后再返回
		DWORD dwRecvBytes = 0;
		if( !socket_provider::singleton().AcceptEx(socket_, sck, p->handler_.address_buffer_, 0,
			details::SOCKET_ADDR_SIZE, details::SOCKET_ADDR_SIZE, &dwRecvBytes, async_result.get()) 
			&& ::WSAGetLastError() != ERROR_IO_PENDING )
			throw service::win32_exception_t("AcceptEx");

		async_result.release();
	}

	// 异步连接服务
	template < typename HandlerT >
	void socket_handle_t::async_connect(const ip_address &addr, u_short uPort, HandlerT &&callback)
	{
		if( !is_open() )
			throw service::network_exception("Socket not open");

		sockaddr_in localAddr		= {0};
		localAddr.sin_family		= AF_INET;

		// 很变态，需要先bind
		int ret = ::bind(socket_, reinterpret_cast<const sockaddr *>(&localAddr), sizeof(localAddr));
		if( ret != 0 )
			throw service::win32_exception_t("bind");

		sockaddr_in remoteAddr		= {0};
		remoteAddr.sin_family		= AF_INET;
		remoteAddr.sin_port			= ::htons(uPort);
		remoteAddr.sin_addr.s_addr	= ::htonl(addr.address());

		typedef detail::connect_handle_t<HandlerT> HookConnect;
		HookConnect connect_hook(*this, std::forward<HandlerT>(callback));
		service::async_callback_base_ptr async_result(service::make_async_callback(std::move(connect_hook)));

		if( !socket_provider::singleton().ConnectEx(socket_, reinterpret_cast<SOCKADDR *>(&remoteAddr), sizeof(SOCKADDR), 0, 0, 0, async_result.get()) 
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("ConnectionEx");

		async_result.release();
	}


	// 异步接接收数据
	template < typename HandlerT >
	void socket_handle_t::async_read(service::mutable_buffer_t &buf, HandlerT &&callback)
	{
		WSABUF wsabuf = {0};
		wsabuf.buf = buf.data();
		wsabuf.len = buf.size();

		DWORD dwFlag = 0;
		DWORD dwSize = 0;

		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback)));

		int ret = ::WSARecv(socket_, &wsabuf, 1, &dwSize, &dwFlag, asynResult.get(), NULL);
		if( 0 != ret
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSARecv");
		//else if( ret == 0 )
		//	asynResult->invoke(std::make_error_code((std::errc::errc)::WSAGetLastError()), dwSize);
		else
			asynResult.release();
	}

	// 异步发送数据
	template < typename HandlerT >
	void socket_handle_t::async_write(const service::const_buffer_t &buf, HandlerT &&callback)
	{
		WSABUF wsabuf = {0};
		wsabuf.buf = const_cast<char *>(buf.data());
		wsabuf.len = buf.size();

		DWORD dwFlag = 0;
		DWORD dwSize = 0;

		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback)));

		int ret = ::WSASend(socket_, &wsabuf, 1, &dwSize, dwFlag, asynResult.get(), NULL);
		if( 0 != ret
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSASend");
		//else if( ret == 0 )
		//	asynResult->invoke(std::make_error_code((std::errc::errc)::WSAGetLastError()), dwSize);
		else
			asynResult.release();
	}

	template < typename HandlerT >
	void socket_handle_t::async_write(const service::const_array_buffer_t &buf, HandlerT &&callback)
	{
		auto async_callback_val = service::make_async_callback(std::forward<HandlerT>(callback));
		service::async_callback_base_ptr asynResult(async_callback_val);

		_async_write_impl(buf, asynResult);
	}

	template < typename ParamT >
	void socket_handle_t::async_write(ParamT &&param, 
		typename std::enable_if<ParamT::is_static_param_t::value>::type * /* = nullptr*/)
	{
		typedef ParamT param_t;
		auto async_callback_val = service::make_async_callback(std::forward<param_t>(param));
		service::async_callback_base_ptr asynResult(async_callback_val);

		auto buffer = async_callback_val->handler_.buffers();
		typedef decltype(buffer) buffer_t;
		WSABUF wsa_buffers[param_t::PARAM_SIZE] = {0};

		for(std::uint32_t i = 0; i != buffer.size(); ++i)
		{
			wsa_buffers[i].buf = const_cast<char *>(buffer[i].data());
			wsa_buffers[i].len = buffer[i].size();
		}

		DWORD dwFlag = 0;
		DWORD dwSize = 0;
		
		int ret = ::WSASend(socket_, &wsa_buffers[0], param_t::PARAM_SIZE, &dwSize, dwFlag, asynResult.get(), NULL);
		if( 0 != ret
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSASend");
		//else if( ret == 0 )
		//	asynResult->invoke(std::make_error_code((std::errc::errc)::WSAGetLastError()), dwSize);
		else
			asynResult.release();
	}


	template < typename ParamT >
	void socket_handle_t::async_write(ParamT &&param, 
		typename std::enable_if<!ParamT::is_static_param_t::value>::type * /* = nullptr*/)
	{
		typedef ParamT param_t;
		
		auto async_callback_val = service::make_async_callback(std::forward<param_t>(param));
		service::async_callback_base_ptr asynResult(async_callback_val);

		_async_write_impl(async_callback_val->handler_.buffers(), asynResult);
	}

	// 异步关闭连接
	template < typename HandlerT >
	void socket_handle_t::async_disconnect(bool is_reuse, HandlerT &&callback)
	{
		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback)));

		DWORD dwFlags = is_reuse ? TF_REUSE_SOCKET : 0;

		if( !socket_provider::singleton().DisconnectEx(socket_, asynResult.get(), dwFlags, 0) 
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("DisConnectionEx");

		asynResult.release();
	}



	// 异步UDP写出
	template < typename HandlerT >
	void socket_handle_t::async_send_to(const service::const_buffer_t &buf, const SOCKADDR_IN *addr, HandlerT &&callback)
	{
		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback)));

		WSABUF wsabuf = {0};
		wsabuf.buf = const_cast<char *>(buf.data());
		wsabuf.len = buf.size();

		DWORD dwFlag = 0;
		DWORD dwSize = 0;

		int ret = ::WSASendTo(socket_, &wsabuf, 1, &dwSize, dwFlag, reinterpret_cast<const sockaddr *>(addr), addr == 0 ? 0 : sizeof(*addr), asynResult.get(), NULL);
		if( 0 != ret
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSASendTo");
		//else if( ret == 0 )
		//	asynResult->invoke(std::make_error_code((std::errc::errc)::WSAGetLastError()), dwSize);
		else
			asynResult.release();
	}	

	// 异步UDP读入
	template < typename HandlerT >
	void socket_handle_t::async_recv_from(service::mutable_buffer_t &buf, SOCKADDR_IN *addr, HandlerT &&callback)
	{
		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback)));

		WSABUF wsabuf = {0};
		wsabuf.buf = buf.data();
		wsabuf.len = buf.size();

		DWORD dwFlag = 0;
		DWORD dwSize = 0;
		int addrLen = (addr == 0 ? 0 : sizeof(addr));

		int ret = ::WSARecvFrom(socket_, &wsabuf, 1, &dwSize, &dwFlag, reinterpret_cast<sockaddr *>(addr), &addrLen, asynResult.get(), NULL);
		if( 0 != ret
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSARecvFrom");
		//else if( ret == 0 )
		//	asynResult->invoke(std::make_error_code((std::errc::errc)::WSAGetLastError()), dwSize);
		else
			asynResult.release();
	}
}
}



#endif