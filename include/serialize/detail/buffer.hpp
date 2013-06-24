#ifndef __SERAILIZE_MEMORY_BUFFER_HPP
#define __SERAILIZE_MEMORY_BUFFER_HPP


#include <fstream>

#pragma warning(disable: 4996)

#ifdef max
#undef max
#endif


namespace serialize
{
	namespace detail
	{

		template < typename CharT >
		class memory_t
		{
		public:
			typedef CharT				value_type;
			typedef CharT *				pointer;
			typedef value_type &		reference;
			typedef const CharT *		const_pointer;
			typedef const value_type &	const_reference;

		private:
			pointer buf_;
			const size_t bufLen_;
			size_t data_len_;

		public:
			explicit memory_t(pointer buf)
				: buf_(buf)
				, bufLen_(std::numeric_limits<size_t>::max())
				, data_len_(0)
			{}

			memory_t(pointer buf, size_t len)
				: buf_(buf)
				, bufLen_(len)
				, data_len_(0)
			{}

			template < size_t N >
			explicit memory_t(value_type (&buf)[N])
				: buf_(buf)
				, bufLen_(N)
				, data_len_(0)
			{}

		public:
			const_pointer buffer() const
			{
				return buf_;
			}

			size_t buffer_length() const
			{
				return bufLen_;
			}

			size_t data_length() const
			{
				return data_len_;
			}

			void read(pointer buf, size_t len, size_t pos) const
			{
				::memmove(buf, buf_ + pos, len);
			}

			void write(const_pointer buf, size_t len, size_t pos)
			{
				data_len_ += len;
				::memmove(buf_ + pos, buf, len);
			}
		};


		class file_t
		{
		public:
			typedef char				value_type;
			typedef char *				pointer;
			typedef value_type &		reference;
			typedef const char *		const_pointer;
			typedef const value_type &	const_reference;

		private:
			std::basic_fstream<value_type> file_;

		public:
			template < typename PathT >
			explicit file_t(const PathT *path, 
				std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out | std::ios_base::binary,
				int prot = std::ios_base::_Openprot)
			{
				file_.open(path, mode, prot);
				if( !file_.good() )
				{
					{
						std::ofstream out(path);
					}
					file_.open(path, mode, prot);
					assert(file_.good());
				}
			}

			~file_t()
			{

			}

		public:
			/*const_pointer Buffer() const
			{
			return file_.rdbuf()->;
			}*/

			size_t buffer_length() const
			{
				return std::numeric_limits<size_t>::max();
			}

			void read(pointer buf, size_t len, size_t pos)
			{
				assert(file_.good());
				if( !file_.good() )
					throw std::runtime_error("file not good");

				file_.seekp(pos);
				file_.read(buf, len);
			}

			void write(const_pointer buf, size_t len, size_t pos)
			{
				assert(file_.good());
				if( !file_.good() )
					throw std::runtime_error("file not good");

				file_.seekg(pos);
				file_.write(buf, len);
			}
		};
	}
}


#endif