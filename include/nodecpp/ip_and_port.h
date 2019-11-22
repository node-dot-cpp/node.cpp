/* -------------------------------------------------------------------------------
* Copyright (c) 2018, OLogN Technologies AG
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the OLogN Technologies AG nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
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

#ifndef IP_AND_PORT_H
#define IP_AND_PORT_H

#include "common.h"

namespace nodecpp {
	
	class Ip4
	{
		uint32_t ip = -1;
		Ip4(uint32_t ip) :ip(ip) {}
	public:
		Ip4() {}
		Ip4(const Ip4&) = default;
		Ip4(Ip4&&) = default;
		Ip4& operator=(const Ip4&) = default;
		Ip4& operator=(Ip4&&) = default;	
	
		uint32_t getNetwork() const { return ip; }
		static Ip4 parse(const char* ip);
		static Ip4 fromNetwork(uint32_t ip);
		nodecpp::string toStr() const;

		bool operator == ( const Ip4& other ) { return ip == other.ip; }
	};
	
	class Port
	{
		uint16_t port = -1;
		Port(uint16_t port) :port(port) {}
	public:
		Port() {}
		Port(const Port&) = default;
		Port(Port&&) = default;
		Port& operator=(const Port&) = default;
		Port& operator=(Port&&) = default;
	
		uint16_t getNetwork() const { return port; }
		uint16_t getHost() const;
		static Port fromHost(uint16_t port);
		static Port fromNetwork(uint16_t port);
		nodecpp::string toStr() const;
	};

	class IPFAMILY
	{
	public:
		enum Value { Undefined, IPv4, IPv6 };
		IPFAMILY() { value_ = Value::Undefined; }
		IPFAMILY(nodecpp::string_literal family) { value_ = strncmp( family.c_str(), "ipv4", 4 ) == 0 ? IPFAMILY::IPv4 : ( ( strncmp( family.c_str(), "ipv6", 4 ) == 0 ) ? IPFAMILY::IPv6 : IPFAMILY::Undefined ); }
		IPFAMILY( IPFAMILY::Value family ) { value_ = family; }
		IPFAMILY( const IPFAMILY& other ) = default;
		IPFAMILY& operator = ( const IPFAMILY& other ) = default;
		IPFAMILY& operator = (nodecpp::string_literal family)  { value_ = strncmp( family.c_str(), "ipv4", 4 ) == 0 ? IPFAMILY::IPv4 : ( ( strncmp( family.c_str(), "ipv6", 4 ) == 0 ) ? IPFAMILY::IPv6 : IPFAMILY::Undefined ); return *this; }
		IPFAMILY( IPFAMILY&& other ) = default;
		IPFAMILY& operator = ( IPFAMILY&& other ) = default;

		bool operator == ( const IPFAMILY& other ) { return value_ == other.value_; }
		bool operator != ( const IPFAMILY& other ) { return value_ != other.value_; }

		nodecpp::string_literal toString( Value family ) { 
			switch ( family )
			{
				case Value::IPv4: return "IPv4";
				case Value::IPv6: return "IPv6";
				case Value::Undefined: return "Undefined";
			}
		}

		Value value() { return value_; }

		void fromNum( uint32_t value ) {
			switch ( value )
			{
				case Value::IPv4: value = Value::IPv4; break;
				case Value::IPv6: value = Value::IPv6; break;
				default: value = Value::Undefined; break;
			}
		}

	private:
		Value value_;
	};


} // namespace nodecpp

#endif //IP_AND_PORT_H
