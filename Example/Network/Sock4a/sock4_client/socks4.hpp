#ifndef SOCKS4_HPP
#define SOCKS4_HPP

#include <string>
#include <array>
#include <cstdint>

#include <iocp/buffer.hpp>

namespace socks4 
{
	using namespace async;

	const unsigned char version = 0x04;

	class request
	{
	private:
		std::uint8_t version_;
		std::uint8_t command_;
		std::uint16_t port_;
		std::uint32_t address_;
		std::string user_id_;
		std::uint8_t null_byte_;

	public:
		enum command_type
		{
			connect = 0x01,
			bind = 0x02
		};

		request(command_type cmd, std::uint16_t port, const std::uint32_t& address, const std::string& user_id)
			: version_(version)
			, command_(cmd)
			, port_(port)
			, address_(address)
			, user_id_(user_id)
			, null_byte_(0)
		{
			
		}

		std::array<iocp::const_buffer, 7> buffers() const
		{
			std::array<iocp::const_buffer, 7> bufs =
			{
				{
					iocp::buffer((const char *)&version_, sizeof(version_)),
					iocp::buffer((const char *)&command_, sizeof(command_)),
					iocp::buffer((const char *)&port_, sizeof(port_)),
					iocp::buffer((const char *)&address_, sizeof(address_)),
					iocp::buffer(user_id_),
					iocp::buffer((const char *)&null_byte_, sizeof(null_byte_))
				}
			};
			return bufs;
		}	
	};



	class reply
	{
	private:
		std::uint8_t null_byte_;
		std::uint8_t status_;
		std::uint8_t port_;
		std::uint32_t address_;

	public:
		enum status_type
		{
			request_granted = 0x5a,
			request_failed = 0x5b,
			request_failed_no_identd = 0x5c,
			request_failed_bad_user_id = 0x5d
		};

		reply()
			: null_byte_(0)
			, status_(0)
		{
		}

		std::array<iocp::mutable_buffer, 5> buffers()
		{
			std::array<iocp::mutable_buffer, 5> bufs =
			{
				{
					iocp::buffer((char *)&null_byte_, sizeof(null_byte_)),
					iocp::buffer((char *)&status_, sizeof(status_)),
					iocp::buffer((char *)&port_, sizeof(port_)),
					iocp::buffer((char *)&address_, sizeof(address_))
				}
			};
			return bufs;
		}

		bool success() const
		{
			return null_byte_ == 0 && status_ == request_granted;
		}

		std::uint8_t status() const
		{
			return status_;
		}

		std::pair<std::uint16_t, std::uint32_t> endpoint() const
		{
			return std::make_pair(port_, address_);
		}

	
	};

} // namespace socks4

#endif