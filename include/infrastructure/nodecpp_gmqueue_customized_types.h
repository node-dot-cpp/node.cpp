#ifndef NODECPP_GMQUEUE_CUSTOMIZED_TYPES_H
#define NODECPP_GMQUEUE_CUSTOMIZED_TYPES_H

#include <marshalling.h>
#include <internal_msg.h>

class GMQueueStatePublisherSubscriberTypeInfo
{
public:
	using BufferT = nodecpp::platform::internal_msg::InternalMsg;
	using ParserT = globalmq::marshalling::JsonParser<BufferT>;
	using ComposerT = globalmq::marshalling::JsonComposer<BufferT>;
//	using SubscriberT = globalmq::marshalling::StateSubscriberBase<BufferT>;
//	using PublisherT = globalmq::marshalling::StatePublisherBase<ComposerT>;
};


#endif // NODECPP_GMQUEUE_CUSTOMIZED_TYPES_H
