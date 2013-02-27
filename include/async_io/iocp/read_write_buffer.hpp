#ifndef __IOCP_READ_WRITE_BUFFER_HPP
#define __IOCP_READ_WRITE_BUFFER_HPP


#include <vector>
#include <string>
#include <array>
#include <type_traits>


namespace async
{
	namespace iocp
	{

		// ---------------------------------------------------------
		// class mutable_buffer

		// 提供安全的修改，由外界提供缓冲区
		class mutable_buffer
		{
		public:
			typedef char					value_type;
			typedef char *					pointer;
			typedef const char*				const_pointer;
			typedef value_type*				iterator;
			typedef const value_type*		const_iterator;

		public:
			value_type *data_;
			size_t size_;

		public:
			mutable_buffer()
				: data_(0)
				, size_(0)
			{}
			mutable_buffer(value_type *data, size_t size)
				: data_(data)
				, size_(size)
			{}

			mutable_buffer &operator=(const mutable_buffer &rhs)
			{
				if( &rhs != this )
				{
					data_ = rhs.data_;
					size_ = rhs.size_;
				}

				return *this;
			}

		public:
			pointer data()
			{
				return data_;
			}
			const_pointer data() const
			{
				return data_;
			}

			size_t size() const
			{
				return size_;
			}

			iterator begin()
			{
				return data_;
			}
			iterator end()
			{
				return data_ + size_;
			}

			const_iterator begin() const
			{
				return data_;
			}
			const_iterator end() const
			{
				return data_ + size_;
			}
		};

		inline mutable_buffer operator+(const mutable_buffer &buffer, size_t sz)
		{
			if( sz > buffer.size_ )
				return mutable_buffer();

			mutable_buffer::pointer newData = buffer.data_ + sz;
			size_t newSize = buffer.size_ - sz;

			return mutable_buffer(newData, newSize);
		}
		inline mutable_buffer operator+(size_t sz, const mutable_buffer &buffer)
		{
			return buffer + sz;
		}



		// ---------------------------------------------------------
		// class ConstBuffer

		// 不能修改缓冲区，由外界提供缓冲区
		class const_buffer
		{
		public:
			typedef char					value_type;
			typedef char *					pointer;
			typedef const char*				const_pointer;
			typedef value_type*				iterator;
			typedef const value_type*		const_iterator;

		public:
			const value_type *data_;
			size_t size_;

		public:
			const_buffer()
				: data_(0)
				, size_(0)
			{}
			const_buffer(const value_type *data, size_t size)
				: data_(data)
				, size_(size)
			{}
			const_buffer(const mutable_buffer &buf)
				: data_(buf.data())
				, size_(buf.size())
			{}

			const_buffer &operator=(const const_buffer &rhs)
			{
				if( &rhs != this )
				{
					data_ = rhs.data_;
					size_ = rhs.size_;
				}

				return *this;
			}

		public:
			const_pointer data() const
			{
				return data_;
			}

			size_t size() const
			{
				return size_;
			}


			const_iterator begin() const
			{
				return data_;
			}
			const_iterator end() const
			{
				return data_ + size_;
			}
		};


		inline const_buffer operator+(const const_buffer &buffer, size_t sz)
		{
			if( sz > buffer.size_ )
				return const_buffer();

			const_buffer::const_pointer newData = buffer.data_  + sz;
			size_t newSize = buffer.size_ - sz;

			return const_buffer(newData, newSize);
		}
		inline const_buffer operator+(size_t sz, const const_buffer &buffer)
		{
			return buffer + sz;
		}



		// ---------------------------------------------------------
		// class NullBuffer

		class null_buffer
		{
		private:
			mutable_buffer buffer_;

		public:
			typedef mutable_buffer				value_type;
			typedef const mutable_buffer*		const_iterator;

		public:
			const_iterator begin() const
			{
				return &buffer_;
			}

			const_iterator end() const
			{
				return &buffer_;
			}
		};



		// ------------------------------------------------------
		// MutableBuffer helper

		inline mutable_buffer buffer(mutable_buffer &buf)
		{
			return mutable_buffer(buf.data_, buf.size_);
		}

		inline mutable_buffer buffer(char *data, size_t sz)
		{
			return mutable_buffer(data, sz);
		}
		
		template<typename PodT, size_t _N>
		inline mutable_buffer buffer(PodT (&data)[_N])
		{
			static_assert(std::is_pod<PodT>::value, "array template param must be PodT type");
			return mutable_buffer(data, _N * sizeof(PodT));
		}

		template<typename PodT, size_t _N>
		inline mutable_buffer buffer(std::array<PodT, _N> &data)
		{
			static_assert(std::is_pod<PodT>::value, "array template param must be PodT type");
			return mutable_buffer(data.data(), _N * sizeof(PodT));
		}

		template<typename PodT>
		inline mutable_buffer buffer(std::vector<PodT> &data)
		{
			return mutable_buffer(&data[0], data.size() * sizeof(PodT));
		}
		
		// ------------------------------------------------------
		// ConstBuffer helper

		inline const_buffer buffer(const const_buffer &buf)
		{
			return const_buffer(buf.data_, buf.size_);
		}
		inline const_buffer buffer(const char *data, size_t sz)
		{
			return const_buffer(data, sz);
		}

		template<typename PodT, size_t _N>
		inline const_buffer buffer(const PodT (&data)[_N])
		{
			static_assert(std::is_pod<PodT>::value, "array template param must be PodT type");
			return const_buffer(data, _N * sizeof(PodT));
		}

		template<typename PodT, size_t _N>
		inline const_buffer buffer(const std::array<PodT, _N> &data)
		{
			static_assert(std::is_pod<PodT>::value, "array template param must be PodT type");
			return const_buffer(data.data(), _N * sizeof(PodT));
		}

		template<typename PodT>
		inline const_buffer buffer(const std::vector<PodT> &data)
		{
			return const_buffer(&data[0], data.size() * sizeof(PodT));
		}

		inline const_buffer buffer(const std::string &data)
		{
			return const_buffer(data.data(), data.length());
		}	
	}
}




#endif