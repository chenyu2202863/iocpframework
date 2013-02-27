#ifndef __PROTOBUF_CODEC_HPP
#define __PROTOBUF_CODEC_HPP


#include <functional>
#include <memory>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include "../../../../include/IOCP/Buffer.hpp"
#include "../../../../include/Network/TCP.hpp"




// struct ProtobufTransportFormat __attribute__ ((__packed__))
// {
//   int32_t  len;
//   int32_t  nameLen;
//   char     typeName[nameLen];
//   char     protobufData[len-nameLen-8];
//   int32_t  checkSum; // adler32 of nameLen, typeName and protobufData
// }


namespace async
{


	typedef std::tr1::shared_ptr<google::protobuf::Message> MessagePtr;


	namespace detail
	{
		inline google::protobuf::int32 AsInt32(const char* buf)
		{
			google::protobuf::int32 be32 = 0;
			::memcpy(&be32, buf, sizeof(be32));
			return be32;
		}


		const std::string kNoErrorStr		= "NoError";
		const std::string kInvalidLengthStr	= "InvalidLength";
		const std::string kCheckSumErrorStr	= "CheckSumError";
		const std::string kInvalidNameLenStr= "InvalidNameLen";
		const std::string kUnknownMessageTypeStr = "UnknownMessageType";
		const std::string kParseErrorStr	= "ParseError";
		const std::string kUnknownErrorStr	= "UnknownError";

	}


	//
	// FIXME: merge with RpcCodec
	//

	template < typename ImplT >
	class ProtobufCodec
	{
	public:

		enum ErrorCode
		{
			kNoError = 0,
			kInvalidLength,
			kCheckSumError,
			kInvalidNameLen,
			kUnknownMessageType,
			kParseError,
		};

		typedef std::tr1::function<void (ImplT&, const MessagePtr&)> ProtobufMessageCallback;
		typedef std::tr1::function<void (ImplT&, const iocp::ConstBuffer &, ErrorCode)> ErrorCallback;

	private:
		ProtobufMessageCallback messageCallback_;
		ErrorCallback errorCallback_;

		const static int kHeaderLen = sizeof(google::protobuf::int32);
		const static int kMinMessageLen = 2 * kHeaderLen + 2;	// nameLen + typeName + checkSum
		const static int kMaxMessageLen = 64 * 1024 * 1024;		// same as codec_stream.h kDefaultTotalBytesLimit

	public:
		explicit ProtobufCodec(const ProtobufMessageCallback& messageCb)
			: messageCallback_(messageCb)
			, errorCallback_(_DefaultErrorCallback)
		{
		}

		ProtobufCodec(const ProtobufMessageCallback& messageCb, const ErrorCallback& errorCb)
			: messageCallback_(messageCb)
			, errorCallback_(errorCb)
		{
		}

	private:
		ProtobufCodec(const ProtobufCodec &);
		ProtobufCodec &operator=(const ProtobufCodec &);

	public:
		void OnMessage(ImplT& sock, const iocp::ConstBuffer& buf)
		{
			typedef google::protobuf::int32 int32;

			size_t total = buf.size();
			size_t readableSize = total;
			while (readableSize >= kMinMessageLen + kHeaderLen)
			{
				const int32 &len = *(int32*)((buf + (total - readableSize)).data());
				if (len > kMaxMessageLen || len < kMinMessageLen)
				{
					errorCallback_(std::tr1::ref(sock), std::tr1::cref(buf), kInvalidLength);
				}
				else if (readableSize >= google::protobuf::implicit_cast<size_t>(len + kHeaderLen))
				{
					ErrorCode errorCode = kNoError;
					MessagePtr message = Parse((buf + (total - readableSize +kHeaderLen)).data(), len, &errorCode);
					if (errorCode == kNoError && message)
					{
						messageCallback_(std::tr1::ref(sock), std::tr1::ref(message));
						readableSize -= (len + kHeaderLen);
					}
					else
					{
						errorCallback_(std::tr1::ref(sock), std::tr1::cref(buf), errorCode);
					}
				}
				else
				{
					break;
				}
			}
		}

		template < typename SocketT, typename HandlerT >
		void Send(SocketT &sock, const google::protobuf::Message &message, const HandlerT &handler)
		{
			std::string buf;
			FillBuffer(buf, message);

			iocp::AsyncWrite(sock, iocp::Buffer(buf), iocp::TransferAll(), handler);
		}


		static const std::string& ErrorCodeToString(ErrorCode errorCode)
		{
			switch (errorCode)
			{
			case kNoError:
				return detail::kNoErrorStr;
			case kInvalidLength:
				return detail::kInvalidLengthStr;
			case kCheckSumError:
				return detail::kCheckSumErrorStr;
			case kInvalidNameLen:
				return detail::kInvalidNameLenStr;
			case kUnknownMessageType:
				return detail::kUnknownMessageTypeStr;
			case kParseError:
				return detail::kParseErrorStr;
			default:
				return detail::kUnknownErrorStr;
			}
		}

		static void FillBuffer(std::string &buf, const google::protobuf::Message& message)
		{
			typedef google::protobuf::int32 int32;

			const std::string& typeName = message.GetTypeName();
			int32 nameLen = static_cast<int32>(typeName.size()+1);
			int32 be32 = nameLen;

			buf.resize(sizeof(int32) * 2 + nameLen);

			buf.append(reinterpret_cast<char*>(&be32), sizeof be32);
			buf.append(typeName.c_str(), nameLen);
			bool succeed = message.AppendToString(&buf);

			if (succeed)
			{
				const char* begin = buf.c_str() + kHeaderLen;

				// CRC32 or adler32 ??
				int32 checkSum = 0;//adler32(1, reinterpret_cast<const Bytef*>(begin), result.size()-kHeaderLen);
				int32 be32 = checkSum;
				buf.append(reinterpret_cast<char*>(&be32), sizeof(be32));

				int32 len = buf.size() - kHeaderLen;
				int32 &totalLen = *(int32 *)(buf.data());
				totalLen = len;
			}
			else
			{
				buf.clear();
			}
		}

		static google::protobuf::Message* CreateMessage(const std::string& typeName)
		{
			google::protobuf::Message* message = NULL;
			const google::protobuf::Descriptor* descriptor =
				google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
			if (descriptor)
			{
				const google::protobuf::Message* prototype =
					google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
				if (prototype)
				{
					message = prototype->New();
				}
			}
			return message;
		}

		static MessagePtr Parse(const char* buf, int len, ErrorCode* error)
		{
			MessagePtr message;

			typedef google::protobuf::int32 int32;

			// check sum
			int32 expectedCheckSum = async::detail::AsInt32(buf + len - kHeaderLen);
			int32 checkSum = static_cast<int32>(0
				/*::adler32(1,
				reinterpret_cast<const Bytef*>(buf),
				static_cast<int>(len - kHeaderLen))*/);
			if (checkSum == expectedCheckSum)
			{
				// get message type name
				int32 nameLen = async::detail::AsInt32(buf);
				if (nameLen >= 2 && nameLen <= len - 2 * kHeaderLen)
				{
					std::string typeName(buf + kHeaderLen, buf + kHeaderLen + nameLen - 1);
					// create message object
					message.reset(CreateMessage(typeName));
					if (message)
					{
						// parse from buffer
						const char* data = buf + kHeaderLen + nameLen;
						int32 dataLen = len - nameLen - 2 * kHeaderLen;
						if (message->ParseFromArray(data, dataLen))
						{
							*error = kNoError;
						}
						else
						{
							*error = kParseError;
						}
					}
					else
					{
						*error = kUnknownMessageType;
					}
				}
				else
				{
					*error = kInvalidNameLen;
				}
			}
			else
			{
				*error = kCheckSumError;
			}

			return message;
		}


	private:
		static void _DefaultErrorCallback(ImplT& sock, const iocp::ConstBuffer &, ErrorCode)
		{
			sock.Close();
		}
	};

}

#endif  // PROTOBUF_CODEC_H