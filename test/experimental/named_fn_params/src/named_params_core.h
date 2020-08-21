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

#ifndef NAMED_PARAMS_CORE_H
#define NAMED_PARAMS_CORE_H

#include <tuple>
#include <string>

namespace m {

template <typename T, typename Parameter>
class NamedType;

template <typename Parameter>
class NamedTypeBase
{
public:
	explicit NamedTypeBase() {}

	struct argument
	{
		template<typename DataType>
		NamedType<DataType, Parameter> operator=(DataType&& value) const
		{
			return NamedType<DataType, Parameter>(std::forward<DataType>(value));
		}
		argument() = default;
		argument(argument const&) = delete;
		argument(argument&&) = delete;
		argument& operator=(argument const&) = delete;
		argument& operator=(argument&&) = delete;
	};
};


template <typename T, typename Parameter>
class NamedType : public NamedTypeBase<Parameter>
{
private:
	T value_;

public:
	explicit NamedType(T const& value) : value_(value) {}
	explicit NamedType(T&& value) : value_(std::move(value)) {}
	T& get() { return value_; }
	T const& get() const { return value_; }

	using NameBase = NamedTypeBase<Parameter>;

	using NamedTypeBase<Parameter>::argument;
};

template<typename TypeToPick, typename... Types>
TypeToPick pick(Types&&... args)
{
	return std::get<TypeToPick>(std::make_tuple(std::forward<Types>(args)...));
}

} // namespace m

#endif // NAMED_PARAMS_CORE_H
