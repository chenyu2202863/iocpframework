#include "stdafx.h"
#include "Codec.hpp"

#include <google/protobuf/descriptor.h>

#include "../../../../include/Network/TCP.hpp"

//#include <zlib.h>  // adler32

using namespace async;
using namespace iocp;
using namespace network;


namespace async
{


	//void ProtobufCodec::FillBuffer(std::string &buf, const google::protobuf::Message& message)
	//{
	//	typedef google::protobuf::int32 int32;

	//	//assert(buf.size() == 0);

	//	//const std::string& typeName = message.GetTypeName();
	//	//int32 nameLen = google::protobuf::implicit_cast<int32>(typeName.size() + 1);

	//	//size_t pos = 0;
	//	//std::copy(reinterpret_cast<char *>(&nameLen), 
	//	//	reinterpret_cast<char *>(&nameLen) + sizeof(nameLen),
	//	//	buf.begin());
	//	//pos += sizeof(nameLen);

	//	//std::copy(typeName.begin(), typeName.end(), (buf + pos).begin());
	//	//pos += nameLen;
	//	//buf.data()[nameLen - 1] = '0';

	//	//// code copied from MessageLite::SerializeToArray() and MessageLite::SerializePartialToArray().
	//	//GOOGLE_DCHECK(message.IsInitialized()) << InitializationErrorMessage("serialize", message);

	//	//int byte_size = message.ByteSize();
	//	//buf->ensureWritableBytes(byte_size);

	//	//uint8_t* start = reinterpret_cast<uint8_t*>(buf->beginWrite());
	//	//uint8_t* end = message.SerializeWithCachedSizesToArray(start);
	//	//if (end - start != byte_size)
	//	//{
	//	//	ByteSizeConsistencyError(byte_size, message.ByteSize(), static_cast<int>(end - start));
	//	//}
	//	//buf->hasWritten(byte_size);

	//	//int32_t checkSum = static_cast<int32_t>(
	//	//	::adler32(1,
	//	//	reinterpret_cast<const Bytef*>(buf->peek()),
	//	//	static_cast<int>(buf->readableBytes())));
	//	//buf->appendInt32(checkSum);
	//	//assert(buf->readableBytes() == sizeof nameLen + nameLen + byte_size + sizeof checkSum);
	//	//int32_t len = sockets::hostToNetwork32(static_cast<int32_t>(buf->readableBytes()));
	//	//buf->prepend(&len, sizeof len);


	//	buf.resize(kHeaderLen);

	//	const std::string& typeName = message.GetTypeName();
	//	int32 nameLen = static_cast<int32>(typeName.size()+1);
	//	int32 be32 = nameLen;
	//	buf.append(reinterpret_cast<char*>(&be32), sizeof be32);
	//	buf.append(typeName.c_str(), nameLen);
	//	bool succeed = message.AppendToString(&buf);

	//	if (succeed)
	//	{
	//		const char* begin = buf.c_str() + kHeaderLen;

	//		// CRC32 or adler32 ??
	//		int32 checkSum = 0;//adler32(1, reinterpret_cast<const Bytef*>(begin), result.size()-kHeaderLen);
	//		int32 be32 = checkSum;
	//		buf.append(reinterpret_cast<char*>(&be32), sizeof(be32));

	//		int32 len = buf.size() - kHeaderLen;
	//		int32 &totalLen = *(int32 *)(buf.data());
	//		totalLen = len;
	//	}
	//	else
	//	{
	//		buf.clear();
	//	}
	//}

	////
	//// no more google code after this
	////

	////
	//// FIXME: merge with RpcCodec
	////

	//namespace
	//{
	//	const std::string kNoErrorStr		= "NoError";
	//	const std::string kInvalidLengthStr	= "InvalidLength";
	//	const std::string kCheckSumErrorStr	= "CheckSumError";
	//	const std::string kInvalidNameLenStr= "InvalidNameLen";
	//	const std::string kUnknownMessageTypeStr = "UnknownMessageType";
	//	const std::string kParseErrorStr	= "ParseError";
	//	const std::string kUnknownErrorStr	= "UnknownError";
	//}

	//const std::string& ProtobufCodec::ErrorCodeToString(ErrorCode errorCode)
	//{
	//	switch (errorCode)
	//	{
	//	case kNoError:
	//		return kNoErrorStr;
	//	case kInvalidLength:
	//		return kInvalidLengthStr;
	//	case kCheckSumError:
	//		return kCheckSumErrorStr;
	//	case kInvalidNameLen:
	//		return kInvalidNameLenStr;
	//	case kUnknownMessageType:
	//		return kUnknownMessageTypeStr;
	//	case kParseError:
	//		return kParseErrorStr;
	//	default:
	//		return kUnknownErrorStr;
	//	}
	//}

	//void ProtobufCodec::_DefaultErrorCallback(network::Tcp::Socket &sock, const iocp::ConstBuffer &buf, ErrorCode code)
	//{
	//	//LOG_ERROR << "ProtobufCodec::defaultErrorCallback - " << errorCodeToString(errorCode);
	//	
	//	sock.Close();
	//}

	//google::protobuf::int32 AsInt32(const char* buf)
	//{
	//	google::protobuf::int32 be32 = 0;
	//	::memcpy(&be32, buf, sizeof(be32));
	//	return be32;
	//}

	//

	//void ProtobufCodec::OnMessage(network::Tcp::Socket &sock, const iocp::ConstBuffer& buf)
	//{
	//	typedef google::protobuf::int32 int32;
	//
	//	size_t total = buf.size();
	//	size_t readableSize = total;
	//	while (readableSize >= kMinMessageLen + kHeaderLen)
	//	{
	//		const int32 &len = *(int32*)((buf + (total - readableSize)).data());
	//		if (len > kMaxMessageLen || len < kMinMessageLen)
	//		{
	//			errorCallback_(std::tr1::ref(sock), std::tr1::cref(buf), kInvalidLength);
	//		}
	//		else if (readableSize >= google::protobuf::implicit_cast<size_t>(len + kHeaderLen))
	//		{
	//			ErrorCode errorCode = kNoError;
	//			MessagePtr message = Parse((buf + (total - readableSize +kHeaderLen)).data(), len, &errorCode);
	//			if (errorCode == kNoError && message)
	//			{
	//				messageCallback_(std::tr1::ref(sock), std::tr1::ref(message));
	//				readableSize -= (len + kHeaderLen);
	//			}
	//			else
	//			{
	//				errorCallback_(std::tr1::ref(sock), std::tr1::cref(buf), errorCode);
	//			}
	//		}
	//		else
	//		{
	//			break;
	//		}
	//	}
	//}

	//google::protobuf::Message* ProtobufCodec::CreateMessage(const std::string& typeName)
	//{
	//	google::protobuf::Message* message = NULL;
	//	const google::protobuf::Descriptor* descriptor =
	//		google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
	//	if (descriptor)
	//	{
	//		const google::protobuf::Message* prototype =
	//			google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
	//		if (prototype)
	//		{
	//			message = prototype->New();
	//		}
	//	}
	//	return message;
	//}

	//MessagePtr ProtobufCodec::Parse(const char* buf, int len, ErrorCode* error)
	//{
	//	MessagePtr message;

	//	typedef google::protobuf::int32 int32;

	//	// check sum
	//	int32 expectedCheckSum = AsInt32(buf + len - kHeaderLen);
	//	int32 checkSum = static_cast<int32>(0
	//		/*::adler32(1,
	//		reinterpret_cast<const Bytef*>(buf),
	//		static_cast<int>(len - kHeaderLen))*/);
	//	if (checkSum == expectedCheckSum)
	//	{
	//		// get message type name
	//		int32 nameLen = AsInt32(buf);
	//		if (nameLen >= 2 && nameLen <= len - 2 * kHeaderLen)
	//		{
	//			std::string typeName(buf + kHeaderLen, buf + kHeaderLen + nameLen - 1);
	//			// create message object
	//			message.reset(CreateMessage(typeName));
	//			if (message)
	//			{
	//				// parse from buffer
	//				const char* data = buf + kHeaderLen + nameLen;
	//				int32 dataLen = len - nameLen - 2 * kHeaderLen;
	//				if (message->ParseFromArray(data, dataLen))
	//				{
	//					*error = kNoError;
	//				}
	//				else
	//				{
	//					*error = kParseError;
	//				}
	//			}
	//			else
	//			{
	//				*error = kUnknownMessageType;
	//			}
	//		}
	//		else
	//		{
	//			*error = kInvalidNameLen;
	//		}
	//	}
	//	else
	//	{
	//		*error = kCheckSumError;
	//	}

	//	return message;
	//}

}