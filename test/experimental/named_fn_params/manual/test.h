#ifndef marshaling_H
#define marshaling_H

#include <named_params_core.h>

namespace m {

using fifthParam_Type = NamedParameter<struct fifthParam_Struct>;
using firstParam_Type = NamedParameter<struct firstParam_Struct>;
using forthParam_Type = NamedParameter<struct forthParam_Struct>;
using secondParam_Type = NamedParameter<struct secondParam_Struct>;
using sixthParam_Type = NamedParameter<struct sixthParam_Struct>;
using thirdParam_Type = NamedParameter<struct thirdParam_Struct>;
using x_Type = NamedParameter<struct x_Struct>;
using y_Type = NamedParameter<struct y_Struct>;
using z_Type = NamedParameter<struct z_Struct>;

constexpr fifthParam_Type::TypeConverter fifthParam;
constexpr firstParam_Type::TypeConverter firstParam;
constexpr forthParam_Type::TypeConverter forthParam;
constexpr secondParam_Type::TypeConverter secondParam;
constexpr sixthParam_Type::TypeConverter sixthParam;
constexpr thirdParam_Type::TypeConverter thirdParam;
constexpr x_Type::TypeConverter x;
constexpr y_Type::TypeConverter y;
constexpr z_Type::TypeConverter z;

//**********************************************************************
// Message "message_one" Targets: JSON GMQ (6 parameters)
// 1. INTEGER firstParam (DEFAULT: 10)
// 2. CHARACTER_STRING secondParam (DEFAULT: "default_v")
// 3. UINTEGER thirdParam (DEFAULT: 30)
// 4. VECTOR<INTEGER> forthParam (REQUIRED)
// 5. VECTOR<NONEXTENDABLE MESSAGE point> fifthParam
// 6. VECTOR< MESSAGE point3D> sixthParam (REQUIRED)

//**********************************************************************

namespace Message_message_one_defaults {
static constexpr impl::StringLiteralForComposing default_2 = { "default_v", sizeof( "default_v" ) - 1};
} // namespace Message_message_one_defaults

template<typename ... Args>
void message_one_compose(Composer& composer, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, firstParam_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, secondParam_Type::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, thirdParam_Type::Name>;
	using arg_4_type = NamedParameterWithType<impl::VectorOfSympleTypes<impl::SignedIntegralType>, forthParam_Type::Name>;
	using arg_5_type = NamedParameterWithType<impl::VectorOfNonextMessageTypes, fifthParam_Type::Name>;
	using arg_6_type = NamedParameterWithType<impl::VectorOfMessageType, sixthParam_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_4_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_5_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_6_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	switch ( composer.proto )
	{
		case Proto::GMQ:
		{
			impl::gmq::composeParamToGmq<arg_1_type, false, int64_t, int64_t, (int64_t)(10)>(arg_1_type::nameAndTypeID, composer, args...);
			impl::gmq::composeParamToGmq<arg_2_type, false, nodecpp::string, const impl::StringLiteralForComposing*, &Message_message_one_defaults::default_2>(arg_2_type::nameAndTypeID, composer, args...);
			impl::gmq::composeParamToGmq<arg_3_type, false, uint64_t, uint64_t, (uint64_t)(30)>(arg_3_type::nameAndTypeID, composer, args...);
			impl::gmq::composeParamToGmq<arg_4_type, true, uint64_t, uint64_t, (uint64_t)(0)>(arg_4_type::nameAndTypeID, composer, args...);
			impl::gmq::composeParamToGmq<arg_5_type, false, uint64_t, uint64_t, (uint64_t)(0)>(arg_5_type::nameAndTypeID, composer, args...);
			impl::gmq::composeParamToGmq<arg_6_type, true, uint64_t, uint64_t, (uint64_t)(0)>(arg_6_type::nameAndTypeID, composer, args...);

			break;
		}
		case Proto::JSON:
		{
			composer.buff.append( "{\n  ", sizeof("{\n  ") - 1 );
			impl::json::composeParamToJson<arg_1_type, false, int64_t, int64_t, (int64_t)(10)>("firstParam", arg_1_type::nameAndTypeID, composer, args...);
			composer.buff.append( ",\n  ", 4 );
			impl::json::composeParamToJson<arg_2_type, false, nodecpp::string, const impl::StringLiteralForComposing*, &Message_message_one_defaults::default_2>("secondParam", arg_2_type::nameAndTypeID, composer, args...);
			composer.buff.append( ",\n  ", 4 );
			impl::json::composeParamToJson<arg_3_type, false, uint64_t, uint64_t, (uint64_t)(30)>("thirdParam", arg_3_type::nameAndTypeID, composer, args...);
			composer.buff.append( ",\n  ", 4 );
			impl::json::composeParamToJson<arg_4_type, true, int64_t, int64_t, (int64_t)(0)>("forthParam", arg_4_type::nameAndTypeID, composer, args...);
			composer.buff.append( ",\n  ", 4 );
			impl::json::composeParamToJson<arg_5_type, false, int64_t, int64_t, (int64_t)(0)>("fifthParam", arg_5_type::nameAndTypeID, composer, args...);
			composer.buff.append( ",\n  ", 4 );
			impl::json::composeParamToJson<arg_6_type, true, int64_t, int64_t, (int64_t)(0)>("sixthParam", arg_6_type::nameAndTypeID, composer, args...);
			composer.buff.append( "\n}", 2 );
			break;
		}
	}
}

template<typename ... Args>
void message_one_parse(Parser& p, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, firstParam_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, secondParam_Type::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, thirdParam_Type::Name>;
	using arg_4_type = NamedParameterWithType<impl::VectorOfSympleTypes<impl::SignedIntegralType>, forthParam_Type::Name>;
	using arg_5_type = NamedParameterWithType<impl::VectorOfNonextMessageTypes, fifthParam_Type::Name>;
	using arg_6_type = NamedParameterWithType<impl::VectorOfMessageType, sixthParam_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_4_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_5_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_6_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	switch ( p.proto )
	{
		case Proto::GMQ:
		{
			impl::gmq::parseGmqParam<arg_1_type, false>(arg_1_type::nameAndTypeID, p, args...);
			impl::gmq::parseGmqParam<arg_2_type, false>(arg_2_type::nameAndTypeID, p, args...);
			impl::gmq::parseGmqParam<arg_3_type, false>(arg_3_type::nameAndTypeID, p, args...);
			impl::gmq::parseGmqParam<arg_4_type, true>(arg_4_type::nameAndTypeID, p, args...);
			impl::gmq::parseGmqParam<arg_5_type, false>(arg_5_type::nameAndTypeID, p, args...);
			impl::gmq::parseGmqParam<arg_6_type, true>(arg_6_type::nameAndTypeID, p, args...);

			break;
		}
		case Proto::JSON:
		{
			p.skipDelimiter( '{' );
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
				else if ( key == "forthParam" )
					impl::json::parseJsonParam<arg_4_type, false>(arg_4_type::nameAndTypeID, p, args...);
				else if ( key == "fifthParam" )
					impl::json::parseJsonParam<arg_5_type, false>(arg_5_type::nameAndTypeID, p, args...);
				else if ( key == "sixthParam" )
					impl::json::parseJsonParam<arg_6_type, false>(arg_6_type::nameAndTypeID, p, args...);
				p.skipSpacesEtc();
				if ( p.isDelimiter( ',' ) )
				{
					p.skipDelimiter( ',' );
					continue;
				}
				if ( p.isDelimiter( '}' ) )
				{
					p.skipDelimiter( '}' );
					break;
				}
				throw std::exception(); // bad format
			}

			break;
		}
	}
}

//**********************************************************************
// Message "point" NONEXTENDABLE Targets: JSON GMQ (2 parameters)
// 1. INTEGER x (REQUIRED)
// 2. INTEGER y (REQUIRED)

//**********************************************************************

template<typename ... Args>
void point_compose(Composer& composer, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, x_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::SignedIntegralType, y_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	switch ( composer.proto )
	{
		case Proto::GMQ:
		{
			impl::gmq::composeParamToGmq<arg_1_type, true, int64_t, int64_t, (int64_t)(0)>(arg_1_type::nameAndTypeID, composer, args...);
			impl::gmq::composeParamToGmq<arg_2_type, true, int64_t, int64_t, (int64_t)(0)>(arg_2_type::nameAndTypeID, composer, args...);

			break;
		}
		case Proto::JSON:
		{
			composer.buff.append( "{\n  ", sizeof("{\n  ") - 1 );
			impl::json::composeParamToJson<arg_1_type, true, int64_t, int64_t, (int64_t)(0)>("x", arg_1_type::nameAndTypeID, composer, args...);
			composer.buff.append( ",\n  ", 4 );
			impl::json::composeParamToJson<arg_2_type, true, int64_t, int64_t, (int64_t)(0)>("y", arg_2_type::nameAndTypeID, composer, args...);
			composer.buff.append( "\n}", 2 );
			break;
		}
	}
}

template<typename ... Args>
void point_parse(Parser& p, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, x_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::SignedIntegralType, y_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	switch ( p.proto )
	{
		case Proto::GMQ:
		{
			impl::gmq::parseGmqParam<arg_1_type, true>(arg_1_type::nameAndTypeID, p, args...);
			impl::gmq::parseGmqParam<arg_2_type, true>(arg_2_type::nameAndTypeID, p, args...);

			break;
		}
		case Proto::JSON:
		{
			p.skipDelimiter( '{' );
			for ( ;; )
			{
				nodecpp::string key;
				p.readKey( &key );
				if ( key == "x" )
					impl::json::parseJsonParam<arg_1_type, false>(arg_1_type::nameAndTypeID, p, args...);
				else if ( key == "y" )
					impl::json::parseJsonParam<arg_2_type, false>(arg_2_type::nameAndTypeID, p, args...);
				p.skipSpacesEtc();
				if ( p.isDelimiter( ',' ) )
				{
					p.skipDelimiter( ',' );
					continue;
				}
				if ( p.isDelimiter( '}' ) )
				{
					p.skipDelimiter( '}' );
					break;
				}
				throw std::exception(); // bad format
			}

			break;
		}
	}
}

//**********************************************************************
// Message "point3D" NONEXTENDABLE Targets: JSON GMQ (3 parameters)
// 1. INTEGER x (DEFAULT: 0)
// 2. INTEGER y (DEFAULT: 0)
// 3. INTEGER z (DEFAULT: 0)

//**********************************************************************

template<typename ... Args>
void point3D_compose(Composer& composer, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, x_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::SignedIntegralType, y_Type::Name>;
	using arg_3_type = NamedParameterWithType<impl::SignedIntegralType, z_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	switch ( composer.proto )
	{
		case Proto::GMQ:
		{
			impl::gmq::composeParamToGmq<arg_1_type, false, int64_t, int64_t, (int64_t)(0)>(arg_1_type::nameAndTypeID, composer, args...);
			impl::gmq::composeParamToGmq<arg_2_type, false, int64_t, int64_t, (int64_t)(0)>(arg_2_type::nameAndTypeID, composer, args...);
			impl::gmq::composeParamToGmq<arg_3_type, false, int64_t, int64_t, (int64_t)(0)>(arg_3_type::nameAndTypeID, composer, args...);

			break;
		}
		case Proto::JSON:
		{
			composer.buff.append( "{\n  ", sizeof("{\n  ") - 1 );
			impl::json::composeParamToJson<arg_1_type, false, int64_t, int64_t, (int64_t)(0)>("x", arg_1_type::nameAndTypeID, composer, args...);
			composer.buff.append( ",\n  ", 4 );
			impl::json::composeParamToJson<arg_2_type, false, int64_t, int64_t, (int64_t)(0)>("y", arg_2_type::nameAndTypeID, composer, args...);
			composer.buff.append( ",\n  ", 4 );
			impl::json::composeParamToJson<arg_3_type, false, int64_t, int64_t, (int64_t)(0)>("z", arg_3_type::nameAndTypeID, composer, args...);
			composer.buff.append( "\n}", 2 );
			break;
		}
	}
}

template<typename ... Args>
void point3D_parse(Parser& p, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, x_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::SignedIntegralType, y_Type::Name>;
	using arg_3_type = NamedParameterWithType<impl::SignedIntegralType, z_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	switch ( p.proto )
	{
		case Proto::GMQ:
		{
			impl::gmq::parseGmqParam<arg_1_type, false>(arg_1_type::nameAndTypeID, p, args...);
			impl::gmq::parseGmqParam<arg_2_type, false>(arg_2_type::nameAndTypeID, p, args...);
			impl::gmq::parseGmqParam<arg_3_type, false>(arg_3_type::nameAndTypeID, p, args...);

			break;
		}
		case Proto::JSON:
		{
			p.skipDelimiter( '{' );
			for ( ;; )
			{
				nodecpp::string key;
				p.readKey( &key );
				if ( key == "x" )
					impl::json::parseJsonParam<arg_1_type, false>(arg_1_type::nameAndTypeID, p, args...);
				else if ( key == "y" )
					impl::json::parseJsonParam<arg_2_type, false>(arg_2_type::nameAndTypeID, p, args...);
				else if ( key == "z" )
					impl::json::parseJsonParam<arg_3_type, false>(arg_3_type::nameAndTypeID, p, args...);
				p.skipSpacesEtc();
				if ( p.isDelimiter( ',' ) )
				{
					p.skipDelimiter( ',' );
					continue;
				}
				if ( p.isDelimiter( '}' ) )
				{
					p.skipDelimiter( '}' );
					break;
				}
				throw std::exception(); // bad format
			}

			break;
		}
	}
}


} // namespace m

#endif // marshaling_H
