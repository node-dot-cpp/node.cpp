#ifndef marshaling_H
#define marshaling_H

#include <named_params_core.h>

namespace m {

using firstParam_Type = NamedParameter<struct firstParam_StructStruct>;
using secondParam_Type = NamedParameter<struct secondParam_StructStruct>;
using thirdParam_Type = NamedParameter<struct thirdParam_StructStruct>;

extern const firstParam_Type::TypeConverter firstParam;
extern const secondParam_Type::TypeConverter secondParam;
extern const thirdParam_Type::TypeConverter thirdParam;

//**********************************************************************
// Message "message_one" (3 parameters)
// 1. INTEGER firstParam (DEFAULT: 10)
// 2. CHARACTER_STRING secondParam (DEFAULT: "default_v")
// 3. UINTEGER thirdParam (DEFAULT: 30)

//**********************************************************************

namespace Message_message_one_defaults {
static constexpr impl::StringLiteralForComposing default_2 = { "default_v", sizeof( "default_v" ) - 1};
} // namespace Message_message_one_defaults

template<typename ... Args>
void message_one_compose(Buffer& b, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, firstParam_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, secondParam_Type::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, thirdParam_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	impl::composeParam<arg_1_type, false, int64_t, int64_t, (int64_t)(10)>(arg_1_type::nameAndTypeID, b, args...);
	impl::composeParam<arg_2_type, false, nodecpp::string, const impl::StringLiteralForComposing*, &Message_message_one_defaults::default_2>(arg_2_type::nameAndTypeID, b, args...);
	impl::composeParam<arg_3_type, false, uint64_t, uint64_t, (uint64_t)(30)>(arg_3_type::nameAndTypeID, b, args...);
}

template<typename ... Args>
void message_one_parse(impl::Parser& p, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, firstParam_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, secondParam_Type::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, thirdParam_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	impl::parseParam<arg_1_type, false>(arg_1_type::nameAndTypeID, p, args...);
	impl::parseParam<arg_2_type, false>(arg_2_type::nameAndTypeID, p, args...);
	impl::parseParam<arg_3_type, false>(arg_3_type::nameAndTypeID, p, args...);
}

template<typename ... Args>
void message_one_composeJson(Buffer& b, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, firstParam_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, secondParam_Type::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, thirdParam_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	impl::json::composeParam<arg_1_type, false, int64_t, int64_t, (int64_t)(10)>("firstParam", arg_1_type::nameAndTypeID, b, args...);
	b.append( ",\n  ", 4 );
	impl::json::composeParam<arg_2_type, false, nodecpp::string, const impl::StringLiteralForComposing*, &Message_message_one_defaults::default_2>("secondParam", arg_2_type::nameAndTypeID, b, args...);
	b.append( ",\n  ", 4 );
	impl::json::composeParam<arg_3_type, false, uint64_t, uint64_t, (uint64_t)(30)>("thirdParam", arg_3_type::nameAndTypeID, b, args...);
	b.appendUint8( '\n' );
	b.appendUint8( 0 );
}

template<typename ... Args>
void message_one_parseJson(impl::Parser& p, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, firstParam_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, secondParam_Type::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, thirdParam_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	for ( ;; )
	{
		nodecpp::string key;
		p.readKey( &key );
		if ( key == "firstParam" )
			impl::json::parseJsonParam<arg_1_type, false>(arg_1_type::nameAndTypeID, p, args...);
		else if ( key == "secondParam" )
			impl::json::parseJsonParam<arg_2_type, false>(arg_2_type::nameAndTypeID, p, args...);
		else if ( key == "thirdParam" )
			impl::json::parseJsonParam<arg_3_type, false>(arg_3_type::nameAndTypeID, p, args...);
		p.skipSpacesEtc();
		if ( p.isComma() )
		{
			p.skipComma();
			continue;
		}
		if ( !p.isData() )
			break;
		throw std::exception(); // bad format
	}
}


} // namespace m

#endif // marshaling_H
