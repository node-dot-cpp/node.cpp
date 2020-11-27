#ifndef _test_marshalling_h_guard
#define _test_marshalling_h_guard

#include <marshalling.h>

namespace m {

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

namespace infrastructural {

using ScreenPoint = impl::MessageName<1>;

template<class BufferT, class ... HandlersT >
void handleMessage( BufferT& buffer, HandlersT ... handlers )
{
	uint64_t msgID;

	GmqParser parser( buffer );
	parser.parseUnsignedInteger( &msgID );
	switch ( msgID )
	{
		case ScreenPoint::id: impl::implHandleMessage<ScreenPoint>( parser, handlers... ); break;
	}

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
	static_assert( std::is_base_of<impl::MessageNameBase, msgID>::value );
	m::GmqComposer composer( buffer );
	impl::composeUnsignedInteger( composer, msgID::id );
	if constexpr ( msgID::id == ScreenPoint::id )
		MESSAGE_ScreenPoint_compose( composer, std::forward<Args>( args )... );
	else
		static_assert( std::is_same<impl::MessageNameBase, msgID>::value, "unexpected value of msgID" ); // note: should be just static_assert(false,"..."); but it seems that in this case clang asserts yet before looking at constexpr conditions
}

} // namespace infrastructural 

//**********************************************************************
// STRUCT "ScreenPoint" NONEXTENDABLE Targets: GMQ (2 parameters)
// 1. INTEGER x (REQUIRED)
// 2. INTEGER y (REQUIRED)

//**********************************************************************

template<class ComposerT, typename ... Args>
void STRUCT_ScreenPoint_compose(ComposerT& composer, Args&& ... args)
{
	static_assert( std::is_base_of<ComposerBase, ComposerT>::value, "Composer must be one of GmqComposer<> or JsonComposer<>" );

	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, x_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::SignedIntegralType, y_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	static_assert( ComposerT::proto == Proto::GMQ, "this STRUCT assumes only GMQ protocol" );
	impl::gmq::composeParamToGmq<ComposerT, arg_1_type, true, int64_t, int64_t, (int64_t)(0)>(composer, arg_1_type::nameAndTypeID, args...);
	impl::gmq::composeParamToGmq<ComposerT, arg_2_type, true, int64_t, int64_t, (int64_t)(0)>(composer, arg_2_type::nameAndTypeID, args...);
}

template<class ParserT, typename ... Args>
void STRUCT_ScreenPoint_parse(ParserT& p, Args&& ... args)
{
	static_assert( std::is_base_of<ParserBase, ParserT>::value, "Parser must be one of GmqParser<> or JsonParser<>" );

	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, x_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::SignedIntegralType, y_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	static_assert( ParserT::proto == Proto::GMQ, "this STRUCT assumes only GMQ protocol" );
	impl::gmq::parseGmqParam<ParserT, arg_1_type, false>(p, arg_1_type::nameAndTypeID, args...);
	impl::gmq::parseGmqParam<ParserT, arg_2_type, false>(p, arg_2_type::nameAndTypeID, args...);
}


} // namespace m

#endif // _test_marshalling_h_guard
