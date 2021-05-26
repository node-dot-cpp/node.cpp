#ifndef wg_marshalling_h_ec194e3c_guard
#define wg_marshalling_h_ec194e3c_guard

#include <marshalling.h>
#include <publishable_impl.h>
using namespace globalmq::marshalling;
namespace mtest {

#ifdef METASCOPE_mtest_ALREADY_DEFINED
#error metascope must reside in a single idl file
#endif
#define METASCOPE_mtest_ALREADY_DEFINED

// Useful aliases:
//     (note: since clang apparently too often requires providing template arguments for aliased type ctors we use wrappers instead of type aliasing)
using Buffer = globalmq::marshalling::Buffer;
using FileReadBuffer = globalmq::marshalling::FileReadBuffer;
template<class BufferT>
class GmqComposer : public globalmq::marshalling::GmqComposer<BufferT> { public: GmqComposer( BufferT& buff_ ) : globalmq::marshalling::GmqComposer<BufferT>( buff_ ) {} };
template<class BufferT>
class GmqParser : public globalmq::marshalling::GmqParser<BufferT> { public: /*GmqParser( BufferT& buff_ ) : globalmq::marshalling::GmqParser<BufferT>( buff_ ) {}*/ GmqParser( typename BufferT::ReadIteratorT& iter ) : globalmq::marshalling::GmqParser<BufferT>( iter ) {} GmqParser( const GmqParser<BufferT>& other ) : globalmq::marshalling::GmqParser<BufferT>( other ) {} GmqParser& operator = ( const GmqParser<BufferT>& other ) { globalmq::marshalling::GmqParser<BufferT>::operator = ( other ); return *this; }};
template<class BufferT>
class JsonComposer : public globalmq::marshalling::JsonComposer<BufferT> { public: JsonComposer( BufferT& buff_ ) : globalmq::marshalling::JsonComposer<BufferT>( buff_ ) {} };
template<class BufferT>
class JsonParser : public globalmq::marshalling::JsonParser<BufferT> { public: /*JsonParser( BufferT& buff_ ) : globalmq::marshalling::JsonParser<BufferT>( buff_ ) {}*/ JsonParser( typename BufferT::ReadIteratorT& iter ) : globalmq::marshalling::JsonParser<BufferT>( iter ) {} JsonParser( const JsonParser<BufferT>& other ) : globalmq::marshalling::JsonParser<BufferT>( other ) {} JsonParser& operator = ( const JsonParser<BufferT>& other ) { globalmq::marshalling::JsonParser<BufferT>::operator = ( other ); return *this; } };
template<class T>
class SimpleTypeCollectionWrapper : public globalmq::marshalling::SimpleTypeCollectionWrapper<T> { public: SimpleTypeCollectionWrapper( T& coll ) : globalmq::marshalling::SimpleTypeCollectionWrapper<T>( coll ) {} };
template<class LambdaSize, class LambdaNext>
class CollectionWrapperForComposing : public globalmq::marshalling::CollectionWrapperForComposing<LambdaSize, LambdaNext> { public: CollectionWrapperForComposing(LambdaSize &&lsize, LambdaNext &&lnext) : globalmq::marshalling::CollectionWrapperForComposing<LambdaSize, LambdaNext>(std::forward<LambdaSize>(lsize), std::forward<LambdaNext>(lnext)) {} };
template<class LambdaCompose>
class MessageWrapperForComposing : public globalmq::marshalling::MessageWrapperForComposing<LambdaCompose> { public: MessageWrapperForComposing(LambdaCompose &&lcompose) : globalmq::marshalling::MessageWrapperForComposing<LambdaCompose>( std::forward<LambdaCompose>(lcompose) ) {} };
template<class LambdaSize, class LambdaNext>
class CollectionWrapperForParsing : public globalmq::marshalling::CollectionWrapperForParsing<LambdaSize, LambdaNext> { public: CollectionWrapperForParsing(LambdaSize &&lsizeHint, LambdaNext &&lnext) : globalmq::marshalling::CollectionWrapperForParsing<LambdaSize, LambdaNext>(std::forward<LambdaSize>(lsizeHint), std::forward<LambdaNext>(lnext)) {} };
template<class LambdaParse>
class MessageWrapperForParsing : public globalmq::marshalling::MessageWrapperForParsing<LambdaParse> { public: MessageWrapperForParsing(LambdaParse &&lparse) : globalmq::marshalling::MessageWrapperForParsing<LambdaParse>(std::forward<LambdaParse>(lparse)) {} };
template<typename msgID_, class LambdaHandler>
MessageHandler<msgID_, LambdaHandler> makeMessageHandler( LambdaHandler &&lhandler ) { return globalmq::marshalling::makeMessageHandler<msgID_, LambdaHandler>(std::forward<LambdaHandler>(lhandler)); }
template<class LambdaHandler>
DefaultMessageHandler<LambdaHandler> makeDefaultMessageHandler( LambdaHandler &&lhandler ) { return globalmq::marshalling::makeDefaultMessageHandler<LambdaHandler>(std::forward<LambdaHandler>(lhandler)); }

//////////////////////////////////////////////////////////////
//
//  Scopes:
//
//  infrastructural
//  {
//    ScreenPoint
//  }
//
//////////////////////////////////////////////////////////////

using x_Type = NamedParameter<struct x_Struct>;
using y_Type = NamedParameter<struct y_Struct>;

constexpr x_Type::TypeConverter x;
constexpr y_Type::TypeConverter y;


// member name presence checkers
template<typename T> concept has_screenPoint_member = requires { { T::screenPoint }; };
template<typename T> concept has_x_member = requires { { T::x }; };
template<typename T> concept has_y_member = requires { { T::y }; };


// member update notifier presence checks
using index_type_for_array_notifiers = size_t&;
template<typename T> concept has_full_update_notifier_call = requires(T t) { { t.notifyFullyUpdated() }; };
template<typename T> concept has_void_update_notifier_call_for_screenPoint = requires(T t) { { t.notifyUpdated_screenPoint() }; };
template<typename StateT, typename MemberT> concept has_update_notifier_call_for_screenPoint = requires { { std::declval<StateT>().notifyUpdated_screenPoint(std::declval<MemberT>()) }; };
template<typename T> concept has_void_update_notifier_call_for_x = requires(T t) { { t.notifyUpdated_x() }; };
template<typename StateT, typename MemberT> concept has_update_notifier_call_for_x = requires { { std::declval<StateT>().notifyUpdated_x(std::declval<MemberT>()) }; };
template<typename T> concept has_void_update_notifier_call_for_y = requires(T t) { { t.notifyUpdated_y() }; };
template<typename StateT, typename MemberT> concept has_update_notifier_call_for_y = requires { { std::declval<StateT>().notifyUpdated_y(std::declval<MemberT>()) }; };

struct publishable_STRUCT_ScreenPoint;
template<class T> class ScreenPoint_RefWrapper;
template<class T, class RootT> class ScreenPoint_RefWrapper4Set;


struct publishable_STRUCT_ScreenPoint : public ::globalmq::marshalling::impl::StructType
{
	template<class ComposerT, class T>
	static
	void compose( ComposerT& composer, const T& t )
	{
		::globalmq::marshalling::impl::publishableStructComposeInteger( composer, t.x, "x", true );

		::globalmq::marshalling::impl::publishableStructComposeInteger( composer, t.y, "y", false );

	}

	template<class ParserT, class T, class RetT = void>
	static
	RetT parse( ParserT& parser, T& t )
	{
		static_assert( std::is_same<RetT, bool>::value || std::is_same<RetT, void>::value );
		constexpr bool reportChanges = std::is_same<RetT, bool>::value;
		bool changed = false;
		static constexpr bool has_void_update_notifier_for_x = has_void_update_notifier_call_for_x<T>;
		static constexpr bool has_update_notifier_for_x = has_update_notifier_call_for_x<T, decltype(T::x)>;
		static constexpr bool has_any_notifier_for_x = has_void_update_notifier_for_x || has_update_notifier_for_x;
		static constexpr bool has_void_update_notifier_for_y = has_void_update_notifier_call_for_y<T>;
		static constexpr bool has_update_notifier_for_y = has_update_notifier_call_for_y<T, decltype(T::y)>;
		static constexpr bool has_any_notifier_for_y = has_void_update_notifier_for_y || has_update_notifier_for_y;
		static constexpr bool has_full_update_notifier = has_full_update_notifier_call<T>;
					if constexpr( has_any_notifier_for_x || reportChanges )
					{
						decltype(T::x) oldVal = t.x;
						::globalmq::marshalling::impl::publishableParseInteger<ParserT, decltype(T::x)>( parser, &(t.x), "x" );
						bool currentChanged = oldVal != t.x;
						if ( currentChanged )
						{
							if constexpr ( reportChanges )
								changed = true;
							if constexpr ( has_void_update_notifier_for_x )
								t.notifyUpdated_x();
							if constexpr ( has_update_notifier_for_x )
								t.notifyUpdated_x( oldVal );
						}
					}
					else
						::globalmq::marshalling::impl::publishableParseInteger<ParserT, decltype(T::x)>( parser, &(t.x), "x" );

					if constexpr( has_any_notifier_for_y || reportChanges )
					{
						decltype(T::y) oldVal = t.y;
						::globalmq::marshalling::impl::publishableParseInteger<ParserT, decltype(T::y)>( parser, &(t.y), "y" );
						bool currentChanged = oldVal != t.y;
						if ( currentChanged )
						{
							if constexpr ( reportChanges )
								changed = true;
							if constexpr ( has_void_update_notifier_for_y )
								t.notifyUpdated_y();
							if constexpr ( has_update_notifier_for_y )
								t.notifyUpdated_y( oldVal );
						}
					}
					else
						::globalmq::marshalling::impl::publishableParseInteger<ParserT, decltype(T::y)>( parser, &(t.y), "y" );


		if constexpr ( reportChanges )
			return changed;
	}

	template<class ParserT, class T, class RetT = void>
	static
	RetT parseForStateSync( ParserT& parser, T& t )
	{
		::globalmq::marshalling::impl::publishableParseInteger<ParserT, decltype(T::x)>( parser, &(t.x), "x" );

		::globalmq::marshalling::impl::publishableParseInteger<ParserT, decltype(T::y)>( parser, &(t.y), "y" );

	}

	template<class ParserT, class T, class RetT = void>
	static
	RetT parse( ParserT& parser, T& t, GMQ_COLL vector<size_t>& addr, size_t offset )
	{
		static_assert( std::is_same<RetT, bool>::value || std::is_same<RetT, void>::value );
		constexpr bool reportChanges = std::is_same<RetT, bool>::value;
		bool changed = false;
		static constexpr bool has_void_update_notifier_for_x = has_void_update_notifier_call_for_x<T>;
		static constexpr bool has_update_notifier_for_x = has_update_notifier_call_for_x<T, decltype(T::x)>;
		static constexpr bool has_any_notifier_for_x = has_void_update_notifier_for_x || has_update_notifier_for_x;
		static constexpr bool has_void_update_notifier_for_y = has_void_update_notifier_call_for_y<T>;
		static constexpr bool has_update_notifier_for_y = has_update_notifier_call_for_y<T, decltype(T::y)>;
		static constexpr bool has_any_notifier_for_y = has_void_update_notifier_for_y || has_update_notifier_for_y;
		static constexpr bool has_full_update_notifier = has_full_update_notifier_call<T>;
		GMQ_ASSERT( addr.size() );
		switch ( addr[offset] )
		{
			case 0:
					if ( addr.size() > offset + 1 )
						throw std::exception(); // bad format, TODO: ...
					if constexpr( has_any_notifier_for_x || reportChanges )
					{
						decltype(T::x) oldVal = t.x;
						::globalmq::marshalling::impl::publishableParseLeafeInteger<ParserT, decltype(T::x)>( parser, &(t.x) );
						bool currentChanged = oldVal != t.x;
						if ( currentChanged )
						{
							if constexpr ( reportChanges )
								changed = true;
							if constexpr ( has_void_update_notifier_for_x )
								t.notifyUpdated_x();
							if constexpr ( has_update_notifier_for_x )
								t.notifyUpdated_x( oldVal );
						}
					}
					else
						::globalmq::marshalling::impl::publishableParseLeafeInteger<ParserT, decltype(T::x)>( parser, &(t.x) );
				break;
			case 1:
					if ( addr.size() > offset + 1 )
						throw std::exception(); // bad format, TODO: ...
					if constexpr( has_any_notifier_for_y || reportChanges )
					{
						decltype(T::y) oldVal = t.y;
						::globalmq::marshalling::impl::publishableParseLeafeInteger<ParserT, decltype(T::y)>( parser, &(t.y) );
						bool currentChanged = oldVal != t.y;
						if ( currentChanged )
						{
							if constexpr ( reportChanges )
								changed = true;
							if constexpr ( has_void_update_notifier_for_y )
								t.notifyUpdated_y();
							if constexpr ( has_update_notifier_for_y )
								t.notifyUpdated_y( oldVal );
						}
					}
					else
						::globalmq::marshalling::impl::publishableParseLeafeInteger<ParserT, decltype(T::y)>( parser, &(t.y) );
				break;
			default:
				throw std::exception(); // unexpected
		}
		if constexpr ( reportChanges )
			return changed;
	}

	template<typename UserT>
	static void copy(const UserT& src, UserT& dst) {
		dst.x = src.x;
		dst.y = src.y;
	}

	template<typename UserT>
	static bool isSame(const UserT& s1, const UserT& s2) {
		if ( s1.x != s2.x ) return false;
		if ( s1.y != s2.y ) return false;
		return true;
	}
};

namespace infrastructural {

using ScreenPoint = ::globalmq::marshalling::impl::MessageName<1>;

template<class ParserT, class ... HandlersT >
void implHandleMessage( ParserT& parser, HandlersT ... handlers )
{
	uint64_t msgID;

	static_assert( ParserT::proto == Proto::GMQ, "According to IDL GMQ parser is expected" );
	parser.parseUnsignedInteger( &msgID );
	switch ( msgID )
	{
		case ScreenPoint::id: ::globalmq::marshalling::impl::implHandleMessage<ScreenPoint>( parser, handlers... ); break;
	}

}

template<class BufferT, class ... HandlersT >
void handleMessage( BufferT& buffer, HandlersT ... handlers )
{
	auto riter = buffer.getReadIter();
	GmqParser<BufferT> parser( riter );
	implHandleMessage( parser, handlers... );
}

template<class ReadIteratorT, class ... HandlersT >
void handleMessage2( ReadIteratorT& riter, HandlersT ... handlers )
{
	GmqParser<typename ReadIteratorT::BufferT> parser( riter );
	implHandleMessage( parser, handlers... );
}

template<typename msgID, class BufferT, typename ... Args>
void composeMessage( BufferT& buffer, Args&& ... args );

//**********************************************************************
// MESSAGE "ScreenPoint" Targets: GMQ (Alias of ScreenPoint)

//**********************************************************************

template<class ComposerT, typename ... Args>
void MESSAGE_ScreenPoint_compose(ComposerT& composer, Args&& ... args)
{
	STRUCT_ScreenPoint_compose(composer, std::forward<Args>( args )...);
}

template<class ParserT, typename ... Args>
void MESSAGE_ScreenPoint_parse(ParserT& p, Args&& ... args)
{
	STRUCT_ScreenPoint_parse(p, std::forward<Args>( args )...);
}

template<typename msgID, class BufferT, typename ... Args>
void composeMessage( BufferT& buffer, Args&& ... args )
{
	static_assert( std::is_base_of<::globalmq::marshalling::impl::MessageNameBase, msgID>::value );
	globalmq::marshalling::GmqComposer composer( buffer );
	::globalmq::marshalling::impl::composeUnsignedInteger( composer, msgID::id );
	if constexpr ( msgID::id == ScreenPoint::id )
		MESSAGE_ScreenPoint_compose( composer, std::forward<Args>( args )... );
	else
		static_assert( std::is_same<::globalmq::marshalling::impl::MessageNameBase, msgID>::value, "unexpected value of msgID" ); // note: should be just static_assert(false,"..."); but it seems that in this case clang asserts yet before looking at constexpr conditions
}

} // namespace infrastructural 

//**********************************************************************
// PUBLISHABLE publishable_sample (1 parameters)
// 1. STRUCT NONEXTENDABLE ScreenPoint screenPoint
//**********************************************************************

template<class T, class ComposerT>
class publishable_sample_WrapperForPublisher : public globalmq::marshalling::StatePublisherBase<ComposerT>
{
	T t;
	using BufferT = typename ComposerT::BufferType;
	BufferT buffer;
	ComposerT composer;
	static constexpr bool has_screenPoint = has_screenPoint_member<T>;
	static_assert( has_screenPoint, "type T must have member T::screenPoint of a type corresponding to IDL type STRUCT ScreenPoint" );


public:
	static constexpr uint64_t numTypeID = 1;
	static constexpr const char* stringTypeID = "publishable_sample";

	template<class ... ArgsT>
	publishable_sample_WrapperForPublisher( ArgsT&& ... args ) : t( std::forward<ArgsT>( args )... ), composer( buffer ) {}
	const T& getState() { return t; }
	ComposerT& getComposer() { return composer; }
	void startTick( BufferT&& buff ) { buffer = std::move( buff ); composer.reset(); ::globalmq::marshalling::impl::composeStateUpdateMessageBegin<ComposerT>( composer );}
	BufferT&& endTick() { ::globalmq::marshalling::impl::composeStateUpdateMessageEnd( composer ); return std::move( buffer ); }
	const char* name() { return stringTypeID; }
	virtual uint64_t stateTypeID() { return numTypeID; }
	const auto& get_screenPoint() { return t.screenPoint; }
	void set_screenPoint( decltype(T::screenPoint) val) { 
		t.screenPoint = val; 
		::globalmq::marshalling::impl::composeAddressInPublishable( composer, GMQ_COLL vector<size_t>(), 0 );
		::globalmq::marshalling::impl::publishableComposeLeafeStructBegin( composer );
		publishable_STRUCT_ScreenPoint::compose( composer, t.screenPoint );
		::globalmq::marshalling::impl::publishableComposeLeafeStructEnd( composer );
	}
	auto get4set_screenPoint() { return ScreenPoint_RefWrapper4Set<decltype(T::screenPoint), publishable_sample_WrapperForPublisher>(t.screenPoint, *this, GMQ_COLL vector<size_t>(), 0); }

	template<class ComposerType>
	void compose( ComposerType& composer )
	{
		::globalmq::marshalling::impl::composeStructBegin( composer );

		::globalmq::marshalling::impl::composePublishableStructBegin( composer, "screenPoint" );
		publishable_STRUCT_ScreenPoint::compose( composer, t.screenPoint );
		::globalmq::marshalling::impl::composePublishableStructEnd( composer, false );


		::globalmq::marshalling::impl::composeStructEnd( composer );
	}
};

template<class T, class RegistrarT>
class publishable_sample_NodecppWrapperForPublisher : public publishable_sample_WrapperForPublisher<T, typename PublisherSubscriberInfo::ComposerT>
{
	using ComposerT = typename PublisherSubscriberInfo::ComposerT;
	RegistrarT& registrar;
public:
	using BufferT = typename PublisherSubscriberInfo::ComposerT::BufferType;
	template<class ... ArgsT>
	publishable_sample_NodecppWrapperForPublisher( RegistrarT& registrar_, ArgsT&& ... args ) : publishable_sample_WrapperForPublisher<T, typename PublisherSubscriberInfo::ComposerT>( std::forward<ArgsT>( args )... ), registrar( registrar_ )
	{ 
		registrar.add( this );
	}

	virtual ~publishable_sample_NodecppWrapperForPublisher()
	{ 
		registrar.remove( this );
	}

	virtual void startTick( BufferT&& buff ) { publishable_sample_WrapperForPublisher<T, ComposerT>::startTick( std::move( buff ) ); }
	virtual BufferT&& endTick() { return  publishable_sample_WrapperForPublisher<T, ComposerT>::endTick(); }
	virtual void generateStateSyncMessage(ComposerT& composer) { publishable_sample_WrapperForPublisher<T, ComposerT>::compose(composer); }
	virtual const char* name() { return publishable_sample_WrapperForPublisher<T, ComposerT>::name(); }
};

template<class T, class BufferT>
class publishable_sample_WrapperForSubscriber : public globalmq::marshalling::StateSubscriberBase<BufferT>
{
	T t;
	static constexpr bool has_screenPoint = has_screenPoint_member<T>;
	static_assert( has_screenPoint, "type T must have member T::screenPoint of a type corresponding to IDL type STRUCT ScreenPoint" );

	static constexpr bool has_void_update_notifier_for_screenPoint = has_void_update_notifier_call_for_screenPoint<T>;
	static constexpr bool has_update_notifier_for_screenPoint = has_update_notifier_call_for_screenPoint<T, decltype(T::screenPoint)>;
	static constexpr bool has_any_notifier_for_screenPoint = has_void_update_notifier_for_screenPoint || has_update_notifier_for_screenPoint;
	static constexpr bool has_full_update_notifier = has_full_update_notifier_call<T>;

public:
	static constexpr uint64_t numTypeID = 1;
	static constexpr const char* stringTypeID = "publishable_sample";

	template<class ... ArgsT>
	publishable_sample_WrapperForSubscriber( ArgsT&& ... args ) : t( std::forward<ArgsT>( args )... ) {}
	const T& getState() { return t; }
	virtual void applyGmqMessageWithUpdates( globalmq::marshalling::GmqParser<BufferT>& parser ) { applyMessageWithUpdates(parser); }
	virtual void applyJsonMessageWithUpdates( globalmq::marshalling::JsonParser<BufferT>& parser ) { applyMessageWithUpdates(parser); }
	virtual const char* name() { return stringTypeID; }
	virtual uint64_t stateTypeID() { return numTypeID; }

	template<typename ParserT>
	void applyMessageWithUpdates(ParserT& parser)
	{
		::globalmq::marshalling::impl::parseStateUpdateMessageBegin( parser );
		GMQ_COLL vector<size_t> addr;
		while( ::globalmq::marshalling::impl::parseAddressInPublishable<ParserT, GMQ_COLL vector<size_t>>( parser, addr ) )
		{
			GMQ_ASSERT( addr.size() );
			switch ( addr[0] )
			{
				case 0:
				{
					if ( addr.size() == 1 ) // we have to parse and apply changes of this child
					{
						::globalmq::marshalling::impl::publishableParseLeafeStructBegin( parser );

						if constexpr( has_update_notifier_for_screenPoint )
						{
							decltype(T::screenPoint) temp_screenPoint;
							publishable_STRUCT_ScreenPoint::copy<decltype(T::screenPoint)>( t.screenPoint, temp_screenPoint );
							bool changedCurrent = publishable_STRUCT_ScreenPoint::parse<ParserT, decltype(T::screenPoint), bool>( parser, t.screenPoint );
							if ( changedCurrent )
							{
								if constexpr( has_void_update_notifier_for_screenPoint )
									t.notifyUpdated_screenPoint();
								t.notifyUpdated_screenPoint( temp_screenPoint );
							}
						}
						else if constexpr( has_void_update_notifier_for_screenPoint )
						{
							bool changedCurrent = publishable_STRUCT_ScreenPoint::parse<ParserT, decltype(T::screenPoint), bool>( parser, t.screenPoint );
							if ( changedCurrent )
							{
								t.notifyUpdated_screenPoint();
							}
						}

						else
						{
							publishable_STRUCT_ScreenPoint::parse( parser, t.screenPoint );
						}

						::globalmq::marshalling::impl::publishableParseLeafeStructEnd( parser );
					}
					else // let child continue parsing
					{
						if constexpr( has_update_notifier_for_screenPoint )
						{
							decltype(T::screenPoint) temp_screenPoint;
							publishable_STRUCT_ScreenPoint::copy<decltype(T::screenPoint)>( t.screenPoint, temp_screenPoint );
							bool changedCurrent = publishable_STRUCT_ScreenPoint::parse<ParserT, decltype(T::screenPoint), bool>( parser, t.screenPoint, addr, 1 );
							if ( changedCurrent )
							{
								if constexpr( has_void_update_notifier_for_screenPoint )
									t.notifyUpdated_screenPoint();
								t.notifyUpdated_screenPoint( temp_screenPoint );
							}
						}
						else if constexpr( has_void_update_notifier_for_screenPoint )
						{
							bool changedCurrent = publishable_STRUCT_ScreenPoint::parse<ParserT, decltype(T::screenPoint), bool>( parser, t.screenPoint, addr, 1 );
							if ( changedCurrent )
							{
								t.notifyUpdated_screenPoint();
							}
						}
						else
							publishable_STRUCT_ScreenPoint::parse( parser, t.screenPoint, addr, 1 );
					}
					break;
				}
				default:
					throw std::exception(); // bad format, TODO: ...
			}
			addr.clear();
		}
	}


	template<class ParserT>
	void parseStateSyncMessage( ParserT& parser )
	{
		::globalmq::marshalling::impl::parseStructBegin( parser );

		::globalmq::marshalling::impl::parsePublishableStructBegin( parser, "screenPoint" );
		publishable_STRUCT_ScreenPoint::parse( parser, t.screenPoint );
		::globalmq::marshalling::impl::parsePublishableStructEnd( parser );

		::globalmq::marshalling::impl::parseStructEnd( parser );

		if constexpr ( has_full_update_notifier )
			t.notifyFullyUpdated();
	}
	const auto& get_screenPoint() { return t.screenPoint; }
};

template<class T, class RegistrarT>
class publishable_sample_NodecppWrapperForSubscriber : public publishable_sample_WrapperForSubscriber<T, typename PublisherSubscriberInfo::BufferT>
{
	RegistrarT& registrar;
public:
	template<class ... ArgsT>
	publishable_sample_NodecppWrapperForSubscriber( RegistrarT& registrar_, ArgsT&& ... args ) : publishable_sample_WrapperForSubscriber<T, typename PublisherSubscriberInfo::BufferT>( std::forward<ArgsT>( args )... ), registrar( registrar_ )
	{ 
		registrar.add( this );
	}

	virtual ~publishable_sample_NodecppWrapperForSubscriber()
	{ 
		registrar.remove( this );
	}

	virtual void applyGmqMessageWithUpdates( globalmq::marshalling::GmqParser<typename PublisherSubscriberInfo::BufferT>& parser ) 
	{
		publishable_sample_WrapperForSubscriber<T, typename PublisherSubscriberInfo::BufferT>::applyMessageWithUpdates(parser);
	}

	virtual void applyJsonMessageWithUpdates( globalmq::marshalling::JsonParser<typename PublisherSubscriberInfo::BufferT>& parser )
	{
		publishable_sample_WrapperForSubscriber<T, typename PublisherSubscriberInfo::BufferT>::applyMessageWithUpdates(parser);
	}

	virtual void applyGmqStateSyncMessage( globalmq::marshalling::GmqParser<typename PublisherSubscriberInfo::BufferT>& parser ) 
	{
		publishable_sample_WrapperForSubscriber<T, typename PublisherSubscriberInfo::BufferT>::parseStateSyncMessage(parser);
	}

	virtual void applyJsonStateSyncMessage( globalmq::marshalling::JsonParser<typename PublisherSubscriberInfo::BufferT>& parser )
	{
		publishable_sample_WrapperForSubscriber<T, typename PublisherSubscriberInfo::BufferT>::parseStateSyncMessage(parser);
	}
	virtual const char* name()
	{
		return publishable_sample_WrapperForSubscriber<T, typename PublisherSubscriberInfo::BufferT>::name();
	}
	void subscribe(GMQ_COLL string path)
	{
		registrar.subscribe( this, path );
	}
};

template<class T, class InputBufferT, class ComposerT>
class publishable_sample_WrapperForConcentrator : public globalmq::marshalling::StateConcentratorBase<InputBufferT, ComposerT>
{
	T t;
	using BufferT = typename ComposerT::BufferType;
	static constexpr bool has_screenPoint = has_screenPoint_member<T>;
	static_assert( has_screenPoint, "type T must have member T::screenPoint of a type corresponding to IDL type STRUCT ScreenPoint" );


public:
	static constexpr uint64_t numTypeID = 1;

	publishable_sample_WrapperForConcentrator() {}
	const char* name() {return "publishable_sample";}
	
	// Acting as publisher
	virtual void generateStateSyncMessage( ComposerT& composer ) { compose(composer); }
	template<class ComposerType>
	void compose( ComposerType& composer )
	{
		::globalmq::marshalling::impl::composeStructBegin( composer );

		::globalmq::marshalling::impl::composePublishableStructBegin( composer, "screenPoint" );
		publishable_STRUCT_ScreenPoint::compose( composer, t.screenPoint );
		::globalmq::marshalling::impl::composePublishableStructEnd( composer, false );


		::globalmq::marshalling::impl::composeStructEnd( composer );
	}

	// Acting as subscriber
	virtual void applyGmqMessageWithUpdates( globalmq::marshalling::GmqParser<BufferT>& parser ) { applyMessageWithUpdates(parser); }
	virtual void applyJsonMessageWithUpdates( globalmq::marshalling::JsonParser<BufferT>& parser ) { applyMessageWithUpdates(parser); }
	virtual void applyGmqStateSyncMessage( globalmq::marshalling::GmqParser<BufferT>& parser ) { parseStateSyncMessage(parser); }
	virtual void applyJsonStateSyncMessage( globalmq::marshalling::JsonParser<BufferT>& parser ) { parseStateSyncMessage(parser); }

	template<typename ParserT>
	void applyMessageWithUpdates(ParserT& parser)
	{
		::globalmq::marshalling::impl::parseStateUpdateMessageBegin( parser );
		GMQ_COLL vector<size_t> addr;
		while( ::globalmq::marshalling::impl::parseAddressInPublishable<ParserT, GMQ_COLL vector<size_t>>( parser, addr ) )
		{
			GMQ_ASSERT( addr.size() );
			switch ( addr[0] )
			{
				case 0:
				{
					if ( addr.size() == 1 ) // we have to parse and apply changes of this child
					{
						::globalmq::marshalling::impl::publishableParseLeafeStructBegin( parser );

						publishable_STRUCT_ScreenPoint::parse( parser, t.screenPoint );

						::globalmq::marshalling::impl::publishableParseLeafeStructEnd( parser );
					}
					else // let child continue parsing
					{
						publishable_STRUCT_ScreenPoint::parse( parser, t.screenPoint, addr, 1 );
					}
					break;
				}
				default:
					throw std::exception(); // bad format, TODO: ...
			}
			addr.clear();
		}
	}

	template<class ParserT>
	void parseStateSyncMessage( ParserT& parser )
	{
		::globalmq::marshalling::impl::parseStructBegin( parser );

		::globalmq::marshalling::impl::parsePublishableStructBegin( parser, "screenPoint" );
		publishable_STRUCT_ScreenPoint::parse( parser, t.screenPoint );
		::globalmq::marshalling::impl::parsePublishableStructEnd( parser );

		::globalmq::marshalling::impl::parseStructEnd( parser );
	}
};

//===============================================================================
// Publishable c-structures
// Use them as-is or copy and edit member types as necessary

struct ScreenPoint
{
	int64_t x;
	int64_t y;
};

struct publishable_sample
{
	ScreenPoint screenPoint;
};


//===============================================================================

template<class InputBufferT, class ComposerT>
class StateConcentratorFactory : public ::globalmq::marshalling::StateConcentratorFactoryBase<InputBufferT, ComposerT>
{
public:
	virtual StateConcentratorBase<InputBufferT, ComposerT>* createConcentrator( uint64_t typeID )
	{
		switch( typeID )
		{
			case 1:
				return new publishable_sample_WrapperForConcentrator<publishable_sample, InputBufferT, ComposerT>;
			default:
				return nullptr;
		}
	}
};

//===============================================================================

template<class T>
class ScreenPoint_RefWrapper
{
	T& t;
	static constexpr bool has_x = has_x_member<T>;
	static_assert( has_x, "type T must have member T::x of a type corresponding to IDL type INTEGER" );
	static constexpr bool has_y = has_y_member<T>;
	static_assert( has_y, "type T must have member T::y of a type corresponding to IDL type INTEGER" );


public:
	ScreenPoint_RefWrapper( T& actual ) : t( actual ) {}
	auto get_x() { return t.x; }
	auto get_y() { return t.y; }
};

template<class T, class RootT>
class ScreenPoint_RefWrapper4Set
{
	T& t;
	RootT& root;
	GMQ_COLL vector<size_t> address;
	static constexpr bool has_x = has_x_member<T>;
	static_assert( has_x, "type T must have member T::x of a type corresponding to IDL type INTEGER" );
	static constexpr bool has_y = has_y_member<T>;
	static_assert( has_y, "type T must have member T::y of a type corresponding to IDL type INTEGER" );


public:
	ScreenPoint_RefWrapper4Set( T& actual, RootT& root_, const GMQ_COLL vector<size_t> address_, size_t idx ) : t( actual ), root( root_ ) {
		address = address_;
		address.push_back (idx );
	}
	auto get_x() { return t.x; }
	void set_x( decltype(T::x) val) { 
		t.x = val; 
		::globalmq::marshalling::impl::composeAddressInPublishable( root.getComposer(), address, 0 );
		::globalmq::marshalling::impl::publishableComposeLeafeInteger( root.getComposer(), t.x );
	}
	auto get_y() { return t.y; }
	void set_y( decltype(T::y) val) { 
		t.y = val; 
		::globalmq::marshalling::impl::composeAddressInPublishable( root.getComposer(), address, 1 );
		::globalmq::marshalling::impl::publishableComposeLeafeInteger( root.getComposer(), t.y );
	}
};

//**********************************************************************
// STRUCT "ScreenPoint" NONEXTENDABLE Targets: GMQ (2 parameters)
// 1. INTEGER x (REQUIRED)
// 2. INTEGER y (REQUIRED)

//**********************************************************************

template<class ComposerT, typename ... Args>
void STRUCT_ScreenPoint_compose(ComposerT& composer, Args&& ... args)
{
	static_assert( std::is_base_of<ComposerBase, ComposerT>::value, "Composer must be one of GmqComposer<> or JsonComposer<>" );

	using arg_1_type = NamedParameterWithType<::globalmq::marshalling::impl::SignedIntegralType, x_Type::Name>;
	using arg_2_type = NamedParameterWithType<::globalmq::marshalling::impl::SignedIntegralType, y_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	static_assert( ComposerT::proto == Proto::GMQ, "this STRUCT assumes only GMQ protocol" );
	::globalmq::marshalling::impl::gmq::composeParamToGmq<ComposerT, arg_1_type, true, int64_t, int64_t, (int64_t)(0)>(composer, arg_1_type::nameAndTypeID, args...);
	::globalmq::marshalling::impl::gmq::composeParamToGmq<ComposerT, arg_2_type, true, int64_t, int64_t, (int64_t)(0)>(composer, arg_2_type::nameAndTypeID, args...);
}

template<class ParserT, typename ... Args>
void STRUCT_ScreenPoint_parse(ParserT& p, Args&& ... args)
{
	static_assert( std::is_base_of<ParserBase, ParserT>::value, "Parser must be one of GmqParser<> or JsonParser<>" );

	using arg_1_type = NamedParameterWithType<::globalmq::marshalling::impl::SignedIntegralType, x_Type::Name>;
	using arg_2_type = NamedParameterWithType<::globalmq::marshalling::impl::SignedIntegralType, y_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	static_assert( ParserT::proto == Proto::GMQ, "this STRUCT assumes only GMQ protocol" );
	::globalmq::marshalling::impl::gmq::parseGmqParam<ParserT, arg_1_type, false>(p, arg_1_type::nameAndTypeID, args...);
	::globalmq::marshalling::impl::gmq::parseGmqParam<ParserT, arg_2_type, false>(p, arg_2_type::nameAndTypeID, args...);
}


} // namespace mtest

#endif // wg_marshalling_h_ec194e3c_guard
