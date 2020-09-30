/* -------------------------------------------------------------------------------
* Copyright (c) 2018, OLogN Technologies AG
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*	 * Redistributions of source code must retain the above copyright
*	   notice, this list of conditions and the following disclaimer.
*	 * Redistributions in binary form must reproduce the above copyright
*	   notice, this list of conditions and the following disclaimer in the
*	   documentation and/or other materials provided with the distribution.
*	 * Neither the name of the OLogN Technologies AG nor the
*	   names of its contributors may be used to endorse or promote products
*	   derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL OLogN Technologies AG BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------------------*/

#ifndef TEST_H
#define TEST_H

#include <named_params_core.h>

namespace man {

using namespace m;

constexpr impl::NoDefaultValueType noDefaultValue;
// GENERATED VALUES

using FirstParam = NamedParameter<struct FirstParamTagStruct>;

using SecondParam = NamedParameter<struct SecondParamTagStruct>;

using ThirdParam = NamedParameter<struct ThirdParamTagStruct>;

using ForthParam = NamedParameter<struct ForthParamTagStruct>;
	
using FifthParam = NamedParameter<struct FifthParamTagStruct>;
	
using SixthParam = NamedParameter<struct SixthParamTagStruct>;

using x_Type = NamedParameter<struct x_StructStruct>;

using y_Type = NamedParameter<struct y_StructStruct>;

using z_Type = NamedParameter<struct z_StructStruct>;


constexpr FirstParam::TypeConverter firstParam;
constexpr SecondParam::TypeConverter secondParam;
constexpr ThirdParam::TypeConverter thirdParam;
constexpr ForthParam::TypeConverter forthParam;
constexpr FifthParam::TypeConverter fifthParam;
constexpr SixthParam::TypeConverter sixthParam;
constexpr x_Type::TypeConverter x;
constexpr y_Type::TypeConverter y;
constexpr z_Type::TypeConverter z;

namespace test2Call_C_defaults {
static constexpr impl::StringLiteralForComposing default_2 = { "default_2", sizeof( "default_2" ) - 1};
}

template<typename ... Args>
void message_one_compose(Composer& composer, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::UnsignedIntegralType, FirstParam::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, SecondParam::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, ThirdParam::Name>;
	using arg_4_type = NamedParameterWithType<impl::VectorOfSympleTypes<impl::SignedIntegralType>, ForthParam::Name>;
	using arg_5_type = NamedParameterWithType<impl::VectorOfNonextMessageTypes, FifthParam::Name>;
	using arg_6_type = NamedParameterWithType<impl::VectorOfMessageType, SixthParam::Name>;
	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_4_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_5_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_6_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );
	impl::composeParam<arg_1_type, false, int, int, 10>(arg_1_type::nameAndTypeID, composer, args...);
	impl::composeParam<arg_2_type, false, nodecpp::string, const impl::StringLiteralForComposing*, &test2Call_C_defaults::default_2>(arg_2_type::nameAndTypeID, composer, args...);
//	impl::composeParam<arg_2_type, false, nodecpp::string, const char*, test2Call_C_defaults::predefault_2>(arg_2_type::nameAndTypeID, composer, args...);
	impl::composeParam<arg_3_type, false, int, int, 30>(arg_3_type::nameAndTypeID, composer, args...);
//	impl::composeParam<arg_4_type, true, impl::NoDefaultValueType, impl::NoDefaultValueType, noDefaultValue>(arg_4_type::nameAndTypeID, composer, args...);
	impl::composeParam<arg_4_type, true, int, int, 10>(arg_4_type::nameAndTypeID, composer, args...);
	impl::composeParam<arg_5_type, true, int, int, 10>(arg_5_type::nameAndTypeID, composer, args...);
	impl::composeParam<arg_6_type, true, int, int, 10>(arg_6_type::nameAndTypeID, composer, args...);
}

template<typename ... Args>
void message_one_parse(impl::Parser& p, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::UnsignedIntegralType, FirstParam::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, SecondParam::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, ThirdParam::Name>;
	using arg_4_type = NamedParameterWithType<impl::VectorOfSympleTypes<impl::SignedIntegralType>, ForthParam::Name>;
	using arg_5_type = NamedParameterWithType<impl::VectorOfNonextMessageTypes, FifthParam::Name>;
	using arg_6_type = NamedParameterWithType<impl::VectorOfMessageType, SixthParam::Name>;
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_4_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_5_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_6_type::nameAndTypeID, Args::nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );
	impl::parseParam<arg_1_type, false>(arg_1_type::nameAndTypeID, p, args...);
	impl::parseParam<arg_2_type, false>(arg_2_type::nameAndTypeID, p, args...);
	impl::parseParam<arg_3_type, false>(arg_3_type::nameAndTypeID, p, args...);
	impl::parseParam<arg_4_type, false>(arg_4_type::nameAndTypeID, p, args...);
	impl::parseParam<arg_5_type, false>(arg_5_type::nameAndTypeID, p, args...);
	impl::parseParam<arg_6_type, false>(arg_6_type::nameAndTypeID, p, args...);
}

template<typename ... Args>
void message_one_composeJson(Composer& composer, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::UnsignedIntegralType, FirstParam::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, SecondParam::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, ThirdParam::Name>;
	using arg_4_type = NamedParameterWithType<impl::VectorOfSympleTypes<impl::SignedIntegralType>, ForthParam::Name>;
	using arg_5_type = NamedParameterWithType<impl::VectorOfNonextMessageTypes, FifthParam::Name>;
	using arg_6_type = NamedParameterWithType<impl::VectorOfMessageType, SixthParam::Name>;
	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_4_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_5_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_6_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );
//	b.append( "\"test2Call_C\": {\n  ", sizeof("\"test2Call_C\": {\n  ") - 1 );
	b.append( "{\n  ", sizeof("{\n  ") - 1 );
	impl::json::composeParam<arg_1_type, false, int, int, 10>("arg_1", arg_1_type::nameAndTypeID, composer, args...);
	b.append( ",\n  ", 4 );
	impl::json::composeParam<arg_2_type, false, nodecpp::string, const impl::StringLiteralForComposing*, &test2Call_C_defaults::default_2>("arg_2", arg_2_type::nameAndTypeID, composer, args...);
	b.append( ",\n  ", 4 );
	impl::json::composeParam<arg_3_type, false, int, int, 30>("arg_3", arg_3_type::nameAndTypeID, composer, args...);
	b.append( ",\n  ", 4 );
	impl::json::composeParam<arg_4_type, true, int, int, 10>("arg_4", arg_4_type::nameAndTypeID, composer, args...);
	b.append( ",\n  ", 4 );
	impl::json::composeParam<arg_5_type, true, int, int, 10>("arg_5", arg_5_type::nameAndTypeID, composer, args...);
	b.append( ",\n  ", 4 );
	impl::json::composeParam<arg_6_type, true, int, int, 10>("arg_6", arg_6_type::nameAndTypeID, composer, args...);
	b.append( "\n}", 2 );
	//b.appendUint8( '\n' );
	//b.appendUint8( 0 );
}

template<typename ... Args>
void message_one_parseJson(impl::Parser& p, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::UnsignedIntegralType, FirstParam::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, SecondParam::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, ThirdParam::Name>;
	using arg_4_type = NamedParameterWithType<impl::VectorOfSympleTypes<impl::SignedIntegralType>, ForthParam::Name>;
	using arg_5_type = NamedParameterWithType<impl::VectorOfNonextMessageTypes, FifthParam::Name>;
	using arg_6_type = NamedParameterWithType<impl::VectorOfMessageType, SixthParam::Name>;
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_4_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_5_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_6_type::nameAndTypeID, Args::nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	p.skipDelimiter( '{' );
	for ( ;; )
	{
		nodecpp::string key;
		p.readKey( &key );
		if ( key == "arg_1" )
//		    impl::json::parseParam<NamedParameterWithType<arg_1_type, FirstParam::Name>, false>(arg_1_type::nameAndTypeID, p, args...);
			impl::json::parseJsonParam<arg_1_type, false>(arg_1_type::nameAndTypeID, p, args...);
		else if ( key == "arg_2" )
//		    impl::json::parseParam<NamedParameterWithType<arg_2_type, FirstParam::Name>, false>(arg_2_type::nameAndTypeID, p, args...);
			impl::json::parseJsonParam<arg_2_type, false>(arg_2_type::nameAndTypeID, p, args...);
		else if ( key == "arg_3" )
//		    impl::json::parseParam<NamedParameterWithType<arg_3_type, FirstParam::Name>, false>(arg_3_type::nameAndTypeID, p, args...);
			impl::json::parseJsonParam<arg_3_type, false>(arg_3_type::nameAndTypeID, p, args...);
		else if ( key == "arg_4" )
//		    impl::json::parseParam<NamedParameterWithType<arg_4_type, FirstParam::Name>, false>(arg_4_type::nameAndTypeID, p, args...);
			impl::json::parseJsonParam<arg_4_type, false>(arg_4_type::nameAndTypeID, p, args...);
		else if ( key == "arg_5" )
//		    impl::json::parseParam<NamedParameterWithType<arg_5_type, FirstParam::Name>, false>(arg_5_type::nameAndTypeID, p, args...);
			impl::json::parseJsonParam<arg_5_type, false>(arg_5_type::nameAndTypeID, p, args...);
		else if ( key == "arg_6" )
//		    impl::json::parseParam<NamedParameterWithType<arg_6_type, FirstParam::Name>, false>(arg_6_type::nameAndTypeID, p, args...);
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
}

//**********************************************************************
// Message "point" (2 parameters)
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

	impl::composeParam<arg_1_type, true, int64_t, int64_t, (int64_t)(0)>(arg_1_type::nameAndTypeID, composer, args...);
	impl::composeParam<arg_2_type, true, int64_t, int64_t, (int64_t)(0)>(arg_2_type::nameAndTypeID, composer, args...);
}

template<typename ... Args>
void point_parse(impl::Parser& p, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, x_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::SignedIntegralType, y_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	impl::parseParam<arg_1_type, true>(arg_1_type::nameAndTypeID, p, args...);
	impl::parseParam<arg_2_type, true>(arg_2_type::nameAndTypeID, p, args...);
}

template<typename ... Args>
void point_composeJson(Composer& composer, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, x_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::SignedIntegralType, y_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

	b.append( "{\n  ", sizeof("{\n  ") - 1 );
	impl::json::composeParam<arg_1_type, true, int64_t, int64_t, (int64_t)(0)>("x", arg_1_type::nameAndTypeID, composer, args...);
	b.append( ",\n  ", 4 );
	impl::json::composeParam<arg_2_type, true, int64_t, int64_t, (int64_t)(0)>("y", arg_2_type::nameAndTypeID, composer, args...);
	b.append( "\n}", 2 );
	//b.appendUint8( '\n' );
	//b.appendUint8( 0 );
}

template<typename ... Args>
void point_parseJson(impl::Parser& p, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::SignedIntegralType, x_Type::Name>;
	using arg_2_type = NamedParameterWithType<impl::SignedIntegralType, y_Type::Name>;

	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + 
		isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );

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

	impl::composeParam<arg_1_type, false, int64_t, int64_t, (int64_t)(0)>(arg_1_type::nameAndTypeID, composer, args...);
	impl::composeParam<arg_2_type, false, int64_t, int64_t, (int64_t)(0)>(arg_2_type::nameAndTypeID, composer, args...);
	impl::composeParam<arg_3_type, false, int64_t, int64_t, (int64_t)(0)>(arg_3_type::nameAndTypeID, composer, args...);
}

template<typename ... Args>
void point3D_parse(impl::Parser& p, Args&& ... args)
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

	impl::parseParam<arg_1_type, false>(arg_1_type::nameAndTypeID, p, args...);
	impl::parseParam<arg_2_type, false>(arg_2_type::nameAndTypeID, p, args...);
	impl::parseParam<arg_3_type, false>(arg_3_type::nameAndTypeID, p, args...);
}

template<typename ... Args>
void point3D_composeJson(Composer& composer, Args&& ... args)
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

	b.append( "{\n  ", sizeof("{\n  ") - 1 );
	impl::json::composeParam<arg_1_type, false, int64_t, int64_t, (int64_t)(0)>("x", arg_1_type::nameAndTypeID, composer, args...);
	b.append( ",\n  ", 4 );
	impl::json::composeParam<arg_2_type, false, int64_t, int64_t, (int64_t)(0)>("y", arg_2_type::nameAndTypeID, composer, args...);
	b.append( ",\n  ", 4 );
	impl::json::composeParam<arg_3_type, false, int64_t, int64_t, (int64_t)(0)>("z", arg_3_type::nameAndTypeID, composer, args...);
	b.append( "\n}", 2 );
	//b.appendUint8( '\n' );
	//b.appendUint8( 0 );
}

template<typename ... Args>
void point3D_parseJson(impl::Parser& p, Args&& ... args)
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
}

} // namespace man

#endif // TEST_H
