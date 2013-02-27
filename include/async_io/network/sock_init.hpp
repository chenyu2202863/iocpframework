#ifndef __NETWORK_WINSOCK_INIT_HPP
#define __NETWORK_WINSOCK_INIT_HPP

#include "../iocp/win_exception.hpp"
#include <memory>



namespace async
{

	namespace network
	{
		namespace detail
		{
			//------------------------------------------------------------------
			// class SockInit

			template<int Major = 2, int Minor = 0>
			class sock_init_t
			{
			private:
				static sock_init_t instance_;

				struct sock_init_impl;
				std::shared_ptr<sock_init_impl> ref_;

			private:
				// 执行真实的初始化
				struct sock_init_impl
				{
				private:
					int result_;

				public:
					sock_init_impl()
					{
						WSADATA wsa_data;
						result_ = ::WSAStartup(MAKEWORD(Major, Minor), &wsa_data);
					}

					~sock_init_impl()
					{
						::WSACleanup();
					}

					int result() const
					{
						return result_;
					}

					// Singleton
					static std::shared_ptr<sock_init_impl> instance()
					{
						static std::shared_ptr<sock_init_impl> init(new sock_init_impl);
						return init;
					}
				};

			public:
				sock_init_t()
					: ref_(sock_init_impl::instance())
				{
					if( this != &instance_ && ref_->result() != 0 )
					{
						throw async::iocp::win32_exception("WSAStartup");
					}
				}

				~sock_init_t()
				{
				}

			private:
				sock_init_t(const sock_init_t &);
				sock_init_t &operator=(const sock_init_t &);
			};

			template<int Major, int Minor>
			sock_init_t<Major, Minor> sock_init_t<Major, Minor>::instance_;

		}
	}
}



#endif