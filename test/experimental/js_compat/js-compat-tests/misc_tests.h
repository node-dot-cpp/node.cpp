/* -------------------------------------------------------------------------------
* Copyright (c) 2020, OLogN Technologies AG
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

#ifndef MISC_TESTS_H
#define MISC_TESTS_H

#include <js_compat.h>

using namespace nodecpp::js;

class MiscTests_ : public nodecpp::js::JSModule
{
public:
	nodecpp::js::JSOwnObj miscTests = nodecpp::js::JSObject::makeJSObject();

private:
#if 0
	JSOwnObj object_with_single_type = nodecpp::js::JSObject::makeJSObject({ 
	  { "reset", {0, 0 } },
	  { "bold", {1, 22 } },
	  { "dim", {2, 22 } },
  
	});

	JSOwnObj object_with_misc_types = nodecpp::js::JSObject::makeJSObject({ 
	  { "reset", {0, 0 } },
	  { "bold", {1, 22 } },
	  { "dim", {2, 22 } },
	  { "num", 3 },
	  { "str", "some str" }
  
	});
#endif

	JSOwnObj object_with_explicit_types = nodecpp::js::JSObject::makeJSObject({ 
	  { "reset", JSArray::makeJSArray({0, 0 }) },
	  { "bold", JSArray::makeJSArray({1, 22 }) },
	  { "dim", JSArray::makeJSArray({2, 22 }) },
	  { "num", 3 },
	  { "str", "some str" }
  
	});

/*	  JSOwnObj array_of_arrays = JSObject::makeJSObject({
		  { "key1", "val1" }, 
		  { "key2", JSObject::makeJSObject({ 
			  { "key1-1", "val1-1" } }) }
	  } );*/
	JSOwnObj array_of_arrays = nodecpp::js::JSArray::makeJSArray({ 
	  JSArray::makeJSArray({JSVar("reset"), JSVar(0), JSVar(0) }),
	  JSArray::makeJSArray({"bold", 1, 22 }),
	  JSArray::makeJSArray({"dim", 2, 22 }),
	  JSArray::makeJSArray({"num", 3 }),
	  JSArray::makeJSArray({"str", "some str" }),
	  "just a string",
	  JSObject::makeJSObject({
		  { "key1", "val1" }, 
		  { "key2", JSObject::makeJSObject({ 
			  { "key1-1", "val1-1" } }) }
	  } )
	});

public:
	MiscTests_()
	{
//printf( "\n======\n%s\n=======\n", object_with_single_type.toString().c_str() );
//printf( "\n======\n%s\n=======\n", object_with_misc_types.toString().c_str() );
printf( "\n======\n%s\n=======\n", object_with_explicit_types.toString().c_str() );
printf( "\n======\n%s\n=======\n", array_of_arrays.toString().c_str() );
#if 0
		object_with_single_type.forEach([this](nodecpp::string key) {
		  auto val = object_with_single_type[key];
		  miscTests[ key ] = JSOwnObj( std::move( nodecpp::js::JSArray::makeJSArray() ) ); // TODO: ownership
//printf( "\n======\n%s\n=======\n", styles.toString().c_str() );
		  JSVar style = styles[key];
//		  auto style = object_with_single_type[key];
		  style[ "open" ] = JSVar( nodecpp::format("\\u001b[{}m", val[0].toString() ) );
		  style[ "close" ] = JSVar( nodecpp::format("\\u001b[{}m", val[1].toString() ) );
printf( "\n======\n%s\n=======\n", style.toString().c_str() );
/*		  auto open = style[ "open" ];
		  auto close = style[ "close" ];
		  auto varopen = JSVar( nodecpp::format("\\u001b[{}m", val[0].toString() ) );
printf( "\n======\n%s\n=======\n", varopen.toString().c_str() );
		  auto varclose = JSVar( nodecpp::format("\\u001b[{}m", val[1].toString() ) );
		  open = varopen;
printf( "\n======\n%s\n=======\n", open.toString().c_str() );
		  close = varclose;
printf( "\n======\n%s\n=======\n", style.toString().c_str() );*/
		});
#endif // 0
	}
};

using MiscTests = JSModule2JSVar<&MiscTests_::miscTests>;

#endif // MISC_TESTS_H
