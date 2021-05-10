#ifndef NODECPP_PUBLISH_SUBSCRIBE_SUPPORT_H
#define NODECPP_PUBLISH_SUBSCRIBE_SUPPORT_H

#include <vector>
#include <marshalling.h>
#include <publishable_impl.h>
#include <infrastructure/q_based_infrastructure.h>

class PublisherSubscriberPoolInfo;
//extern thread_local globalmq::marshalling::PublisherPool<PublisherSubscriberPoolInfo> publishers;
//extern thread_local globalmq::marshalling::SubscriberPool<PublisherSubscriberPoolInfo> subscribers;

// transporting level simulation (for this test, single-threaded)
constexpr uint32_t publisherPoolAddress = 1;
constexpr uint32_t subscriberPoolAddress = 0;
//extern GMQ_COLL vector<nodecpp::platform::internal_msg::InternalMsg> likeQueues[2];

class PublisherSubscriberInfo
{
public:
	using BufferT = nodecpp::platform::internal_msg::InternalMsg;
	using ParserT = globalmq::marshalling::JsonParser<BufferT>;
	using ComposerT = globalmq::marshalling::JsonComposer<BufferT>;
	using SubscriberT = globalmq::marshalling::StateSubscriberBase<BufferT>;
	using PublisherT = globalmq::marshalling::StatePublisherBase<ComposerT>;
};

class PublisherSubscriberPoolInfoForWorkerThread : public PublisherSubscriberInfo
{
public:
	// publishers and subscribers

	// addressing (what is kept at publisher's size
	using NodeAddressT = NodeAddress;

	// used by pools
	static void sendSubscriptionRequest( BufferT& buff, GMQ_COLL string publisherName )
	{
		assert( false ); // TODO: revise if this call is actually expected at main thread
	}
	static void sendMsgFromPublisherToSubscriber( BufferT& buff, NodeAddressT subscrAddr )
	{
		internalPostlGlobalMQ( std::move( buff ), subscrAddr );
	}
};

class PublisherSubscriberPoolInfoForMainThread : public PublisherSubscriberInfo
{
public:
	// publishers and subscribers

	// addressing (what is kept at publisher's size
	using NodeAddressT = NodeAddress;

	// used by pools
	static void sendSubscriptionRequest( BufferT& buff, GMQ_COLL string publisherName )
	{
		// Note: we assume that here or around there is a kind of routing table converting publisherName to some deliverable address
		//       here we just emulate it manually
		assert( publisherName == "publishable_sample" );
		NodeAddress publisherAddr({ ThreadID({ 2, 1 }), 0 }); // note
		internalPostlGlobalMQ( std::move( buff ), publisherAddr );
	}
	static void sendMsgFromPublisherToSubscriber( BufferT& buff, NodeAddressT subscrAddr )
	{
		assert( false ); // TODO: revise if this call is actually expected at main thread
	}
};

using PoolForWorkerThreadT = globalmq::marshalling::MetaPool<GMQueueStatePublisherSubscriberTypeInfo>;
using PoolForMainThreadT = globalmq::marshalling::MetaPool<GMQueueStatePublisherSubscriberTypeInfo>;

#endif // NODECPP_PUBLISH_SUBSCRIBE_SUPPORT_H
