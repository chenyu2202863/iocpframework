#ifndef __PROTOBUF_CODEC_DISPATCHER_HPP
#define __PROTOBUF_CODEC_DISPATCHER_HPP


#include <google/protobuf/message.h>

#include <unordered_map>
#include <functional>
#include <memory>
#include <type_traits>

#include "../../../../include/Network/TCP.hpp"


namespace async
{

	namespace dispatch
	{
		typedef std::tr1::shared_ptr<google::protobuf::Message> MessagePtr;

		template < typename ImplT >
		class Callback
		{
		public:
			virtual ~Callback() {};
			virtual void OnMessage(ImplT&, const MessagePtr&) const = 0;
		};


		template < typename ImplT, typename T >
		class CallbackT 
			: public Callback<ImplT>
		{
			GOOGLE_COMPILE_ASSERT((std::tr1::is_base_of<google::protobuf::Message, T>::value), N);

		public:
			typedef std::tr1::function<void (ImplT&,
				const std::tr1::shared_ptr<T>& message)> ProtobufMessageTCallback;

		private:
			ProtobufMessageTCallback callback_;

		public:
			CallbackT(const ProtobufMessageTCallback& callback)
				: callback_(callback)
			{
			}

			virtual void OnMessage(ImplT& sock, const MessagePtr& message) const
			{
				std::tr1::shared_ptr<T> concrete = std::tr1::dynamic_pointer_cast<T>(message);
				assert(concrete != 0);
				callback_(std::tr1::ref(sock), std::tr1::cref(concrete));
			}
		};


		// ---------------------------
		template < typename ImplT >
		class ProtobufDispatcher
		{
		public:
			typedef std::tr1::function<void (ImplT&, const MessagePtr& message)> ProtobufMessageCallback;

		private:
			typedef std::tr1::unordered_map<const google::protobuf::Descriptor*, std::tr1::shared_ptr<Callback<ImplT>> > CallbackMap;

			CallbackMap callbacks_;
			ProtobufMessageCallback defaultCallback_;

		public:
			explicit ProtobufDispatcher(const ProtobufMessageCallback& defaultCb)
				: defaultCallback_(defaultCb)
			{
			}

			void OnProtobufMessage(ImplT& sock, const MessagePtr& message) const
			{
				CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());
				if (it != callbacks_.end())
				{
					it->second->OnMessage(std::tr1::ref(sock), std::tr1::cref(message));
				}
				else
				{
					defaultCallback_(std::tr1::ref(sock), std::tr1::cref(message));
				}
			}

			template< typename T >
			void RegisterMessageCallback(const typename CallbackT<ImplT, T>::ProtobufMessageTCallback& callback)
			{
				std::tr1::shared_ptr<Callback<ImplT>> pd(new CallbackT<ImplT, T>(callback));
				callbacks_[T::descriptor()] = pd;
			}
		};
	}
	

}

#endif