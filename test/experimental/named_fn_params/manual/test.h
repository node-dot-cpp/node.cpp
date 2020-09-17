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
// GENERATED VALUES

using FirstParam = NamedParameter<struct FirstParamTagStruct>;

using SecondParam = NamedParameter<struct SecondParamTagStruct>;

using ThirdParam = NamedParameter<struct ThirdParamTagStruct>;
	
extern const FirstParam::TypeConverter firstParam;
extern const SecondParam::TypeConverter secondParam;
extern const ThirdParam::TypeConverter thirdParam;

namespace test2Call_C_defaults {
static constexpr impl::StringLiteralForComposing default_2 = { "default_2", sizeof( "default_2" ) - 1};
}

template<typename ... Args>
void message_one_compose(Buffer& b, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::UnsignedIntegralType, FirstParam::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, SecondParam::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, ThirdParam::Name>;
	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );
	impl::composeParam<arg_1_type, false, int, int, 10>(arg_1_type::nameAndTypeID, b, args...);
	impl::composeParam<arg_2_type, false, nodecpp::string, const impl::StringLiteralForComposing*, &test2Call_C_defaults::default_2>(arg_2_type::nameAndTypeID, b, args...);
//	impl::composeParam<arg_2_type, false, nodecpp::string, const char*, test2Call_C_defaults::predefault_2>(arg_2_type::nameAndTypeID, b, args...);
	impl::composeParam<arg_3_type, false, int, int, 30>(arg_3_type::nameAndTypeID, b, args...);
}

template<typename ... Args>
void message_one_parse(impl::Parser& p, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::UnsignedIntegralType, FirstParam::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, SecondParam::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, ThirdParam::Name>;
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );
	impl::parseParam<arg_1_type, false>(arg_1_type::nameAndTypeID, p, args...);
	impl::parseParam<arg_2_type, false>(arg_2_type::nameAndTypeID, p, args...);
	impl::parseParam<arg_3_type, false>(arg_3_type::nameAndTypeID, p, args...);
}

template<typename ... Args>
void message_one_composeJson(Buffer& b, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::UnsignedIntegralType, FirstParam::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, SecondParam::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, ThirdParam::Name>;
	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...);
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );
//	b.append( "\"test2Call_C\": {\n  ", sizeof("\"test2Call_C\": {\n  ") - 1 );
	impl::json::composeParam<arg_1_type, false, int, int, 10>("arg_1", arg_1_type::nameAndTypeID, b, args...);
	b.append( ",\n  ", 4 );
	impl::json::composeParam<arg_2_type, false, nodecpp::string, const impl::StringLiteralForComposing*, &test2Call_C_defaults::default_2>("arg_2", arg_2_type::nameAndTypeID, b, args...);
	b.append( ",\n  ", 4 );
	impl::json::composeParam<arg_3_type, false, int, int, 30>("arg_3", arg_3_type::nameAndTypeID, b, args...);
//	b.append( ",\n}\n", 4 );
	b.appendUint8( '\n' );
	b.appendUint8( 0 );
}

template<typename ... Args>
void message_one_parseJson(impl::Parser& p, Args&& ... args)
{
	using arg_1_type = NamedParameterWithType<impl::UnsignedIntegralType, FirstParam::Name>;
	using arg_2_type = NamedParameterWithType<impl::StringType, SecondParam::Name>;
	using arg_3_type = NamedParameterWithType<impl::UnsignedIntegralType, ThirdParam::Name>;
	constexpr size_t argCount = sizeof ... (Args);
	if constexpr ( argCount != 0 )
		ensureUniqueness(args.nameAndTypeID...);
	constexpr size_t matchCount = isMatched(arg_1_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_2_type::nameAndTypeID, Args::nameAndTypeID...) + isMatched(arg_3_type::nameAndTypeID, Args::nameAndTypeID...);
	static_assert( argCount == matchCount, "unexpected arguments found" );
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

} // namespace man

#endif // TEST_H
