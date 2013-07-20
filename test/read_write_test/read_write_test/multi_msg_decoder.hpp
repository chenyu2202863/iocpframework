#ifndef __BAIMO_NETWORK_MULTI_MESSAGE_DECODER_HPP
#define __BAIMO_NETWORK_MULTI_MESSAGE_DECODER_HPP

#include <memory>
#include <algorithm>

#ifdef min
#undef min
#endif

typedef std::shared_ptr<char> buffer_data_t;
typedef std::pair<std::uint32_t, buffer_data_t> buffer_t;
typedef std::shared_ptr<buffer_t> buffer_ptr;

buffer_t make_buffer(std::uint32_t len)
{
	buffer_data_t data(( char * )::operator new( len ), [](char *p){ ::operator delete( p ); });

	return {len, data};
}

namespace baimo { namespace network {

	template < typename SessionT >
	struct multi_buffer_decode_t
	{
		buffer_t tmp_buffer_;
		std::uint32_t tmp_buffer_len_;

		char header_data_[4];
		std::uint32_t header_len_;

		typedef std::function<void(const SessionT &, const buffer_t &) > buffer_callback_t;
		buffer_callback_t buffer_callback_;

		multi_buffer_decode_t(const buffer_callback_t &buffer_callback)
			: tmp_buffer_len_(0)
			, header_len_(0)
			, buffer_callback_(buffer_callback)
		{}

		void handle(const SessionT &session, const char *buffer, std::uint32_t len)
		{
			if( tmp_buffer_.first == 0 )
			{
				if( len < sizeof(std::uint32_t) )
				{
					std::copy(buffer, buffer + len, header_data_);
					header_len_ = len;
					return;
				}

				std::uint32_t data_len = 0;
				if( header_len_ != 0 )
				{
					auto level_header_len = sizeof(std::uint32_t) - header_len_;
					std::copy(buffer, buffer + level_header_len,
							  stdext::make_unchecked_array_iterator(header_data_ + header_len_));
					data_len = *reinterpret_cast<std::uint32_t *>(header_data_);
					header_len_ = 0;

					buffer += level_header_len;
					len -= level_header_len;
				}
				else
				{
					data_len = *reinterpret_cast<const std::uint32_t *>(buffer);
					buffer += sizeof(std::uint32_t);
					len -= sizeof(std::uint32_t);
				}

				buffer_t buffer_data = make_buffer(data_len);
				if( data_len == len )
				{
					std::copy(buffer, buffer + data_len,
							  stdext::make_unchecked_array_iterator(buffer_data.second.get()));
					buffer_callback_(session, buffer_data);
				}
				else if( data_len > len )
				{
					std::copy(buffer, buffer + len,
							  stdext::make_unchecked_array_iterator(buffer_data.second.get()));
					tmp_buffer_len_ = len;
					tmp_buffer_ = buffer_data;
				}
				else
				{
					std::copy(buffer, buffer + data_len,
							  stdext::make_unchecked_array_iterator(buffer_data.second.get()));
					buffer_callback_(session, buffer_data);

					handle(session, buffer + data_len, len - data_len);
				}
			}
			else
			{
				auto over_len = tmp_buffer_.first - tmp_buffer_len_;
				auto copy_len = std::min(len, over_len);
				std::copy(buffer, buffer + copy_len,
						  stdext::make_unchecked_array_iterator(tmp_buffer_.second.get() + tmp_buffer_len_));
				tmp_buffer_len_ += copy_len;

				if( tmp_buffer_len_ == tmp_buffer_.first )
				{
					buffer_callback_(session, tmp_buffer_);
					tmp_buffer_.first = 0;
					tmp_buffer_len_ = 0;
				}

				len -= copy_len;
				if( len > 0 )
					handle(session, buffer + over_len, len);
			}
		}
	};

}
}

#endif