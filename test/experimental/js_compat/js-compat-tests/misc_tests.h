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

#include <nodecpp/js_compat.h>

namespace nodecpp {
	template <class _Ty>
	NODISCARD constexpr std::remove_reference_t<_Ty>&& move(_Ty&& _Arg) noexcept { // forward _Arg as movable
		return static_cast<std::remove_reference_t<_Ty>&&>(_Arg);
	}
} // using namespace nodecpp

using namespace nodecpp::js;


class MiscTests : public nodecpp::js::JSModule
{
public:
	nodecpp::js::JSOwnObj miscTests = makeJSObject();

private:
	JSOwnObj object_with_explicit_types = makeJSObject({ 
	  { "reset", makeJSArray({0, 0 }) },
	  { "bold", makeJSArray({1, 22 }) },
	  { "dim", makeJSArray({2, 22 }) },
	  { "num", 3 },
	  { "str", "some str" }
  
	});

	JSVar likeFn;

	JSOwnObj array_of_arrays = makeJSArray({ 
	  makeJSArray({JSVar("reset"), JSVar(0), JSVar(0) }),
	  makeJSArray({"bold", 1, 22 }),
	  makeJSArray({"dim", 2, 22 }),
	  makeJSArray({"num", 3 }),
	  makeJSArray({"str", "some str" }),
	  "just a string",
	  makeJSObject({
		  { "key1", "val1" }, 
		  { "key2", makeJSObject({ 
			  { "key1-1", "val1-1" } }) }
	  } )
	});

public:
	MiscTests()
	{

		JSOwnObj array_of_arrays1 = makeJSArray({ 
		  makeJSArray({"reset", 0, 0 }),
		  makeJSArray({"bold", 1, 22 }),
		  makeJSArray({"dim", 2, 22 }),
		  makeJSArray({"num", 3 }),
		  makeJSArray({"str", "some str" }),
		  "just a string",
		  makeJSObject({
			  { "key1", "val1" }, 
			  { "key2", makeJSObject({ 
				  { "key1-1", "val1-1" } }) }
		  } )
		});

		JSOwnObj* pobj = &array_of_arrays1;
		nodecpp::safememory::owning_ptr<JSObject>* pown = reinterpret_cast<nodecpp::safememory::owning_ptr<JSObject>*>(pobj);
		JSObject& obj1 = *(*pown);

		JSOwnObj arr1 = makeJSArray({0, 0 });
		JSOwnObj arr2 = makeJSArray({1, 22 });

		JSOwnObj object_with_explicit_types_2 = makeJSObject({ 
		  { "reset", nodecpp::move(arr1) },
		  { "bold", arr2 },
		  { "dim", makeJSArray({2, 22 }) },
		  { "num", 3 },
		  { "str", "some str" },
		  { "getStr", JSVar([]() { return "this is lambda ret"; }) },
		  { "echo", JSVar([](JSVar x) { return x; }) },
		  { "concat2", JSVar([](JSVar x1, JSVar x2) { return x1.toString() + x2.toString(); }) },
		  { "concat3", JSVar([](JSVar x1, JSVar x2, JSVar x3) { return x1.toString() + x2.toString() + x3.toString(); }) },
		  { "concat4", JSVar([](JSVar x1, JSVar x2, JSVar x3, JSVar x4) { return x1.toString() + x2.toString() + x3.toString() + x4.toString(); }) },
		  { "concat_any", JSVar([]() { 
			  nodecpp::string ret = "";
			  JSVar params = arguments();
			  params.forEach( [&ret, params](JSVar idx){
				  ret += params[idx].toString();
				  });
			  return ret;
			  }) },
		  { "prefix adder generator", JSVar([](JSVar prefix) { return JSVar( [prefix](JSVar s){ return prefix.toString() + s.toString();});}) },
  
		});

		arr2[0] = 999;

		JSVar prefixAdder = object_with_explicit_types_2["prefix adder generator"]( "prefix + " );
		JSVar strvar = "some text";
		JSVar arrvar = object_with_explicit_types_2["bold"];
		JSVar numvar = 17;
		JSVar boolvar = true;


		console.log( "\n======\n{}\n=======\n", object_with_explicit_types );
		console.log( "\n======\n{}\n=======\n", array_of_arrays );
		console.log( "\n======\n{}\n=======\n", object_with_explicit_types_2 );
		console.log( "\n======\n{}\n=======\n", object_with_explicit_types_2["getStr"]() );
		console.log( "\n======\n{}\n=======\n", object_with_explicit_types_2["echo"]( "echoing my cry" ) );
		console.log( "\n======\n{}\n=======\n", object_with_explicit_types_2["concat2"]( "word1 ", "word2 " ) );
		console.log( "\n======\n{}\n=======\n", object_with_explicit_types_2["concat3"]( "word1 ", "word2 ", "word3 " ) );
		console.log( "\n======\n{}\n=======\n", object_with_explicit_types_2["concat4"]( "word1 ", "word2 ", "word3 ", "word4 " ) );
		console.log( "\n======\n{}\n=======\n", object_with_explicit_types_2["concat4"]( "word1 ", "word2 ", "word3 " ) );
		console.log( "\n======\n{}\n=======\n", object_with_explicit_types_2["concat4"]( "word1 ", "word2 " ) );
		console.log( "\n======\n{}\n=======\n", object_with_explicit_types_2["concat4"]( "word1 " ) );
		console.log( "\n======\n{}\n=======\n", object_with_explicit_types_2["concat4"]() );
		console.log( "\n======\n{}\n=======\n", object_with_explicit_types_2["concat_any"]( "word1 ", "word2 ", "word3 ", "word4 " ) );
		console.log( "\n======\n{}\n=======\n", prefixAdder( "some text after" ) );

		// misc math, etc
		console.log( "\n======math=======\n" );
		console.log( JSVar(3.5) % JSVar(2));
		console.log( JSVar(-3.5) % JSVar(2));
		console.log( JSVar(7.5) % JSVar(2.4));
		console.log( JSVar("7.5") % JSVar("2.4"));

		console.log( JSVar("abc") || JSVar("def"));
		console.log( JSVar("") || JSVar("def"));
		console.log( JSVar() || JSVar("def"));
		console.log( JSVar(false) || JSVar("def"));

		console.log( JSVar("ghi") && JSVar("jkl"));
		console.log( JSVar("ghi") && JSVar(""));
		console.log( JSVar("") && JSVar("jkl"));
		console.log( JSVar("ghi") && JSVar());
		console.log( JSVar("ghi") && JSVar(false));

		console.log( (JSVar("3"))++);
		console.log( ++(JSVar("3")));
		console.log( (JSVar("3"))--);
		console.log( --(JSVar("3")));

		console.log( (JSVar("abc")).split());
		console.log( (JSVar("abc")).split(""));
		console.log( (JSVar("abc def ghi")).split(" "));
		console.log( (JSVar(" abc def ghi ")).split( " "));

		console.log( (array_of_arrays1["0"].concat( array_of_arrays1["1"], array_of_arrays1["2"] )));
		console.log( ((makeJSArray())->concat(array_of_arrays1["0"], array_of_arrays1["1"], array_of_arrays1["2"] )));

		console.log( "\n======\n{}\n=======\n", array_of_arrays1.length() );
		console.log( "\n======\n{}\n=======\n", array_of_arrays1["0"].length() );

		console.log(JSMath::floor(JSVar(3.14)));
		console.log(JSMath::floor(JSVar(-3.14)));

		console.log( JSMath::random() );
		console.log( JSMath::random() );
		console.log( JSMath::random() );

		console.log( (JSVar("3_AbCd, #")).toLowerCase() );
		console.log( (JSVar("3_AbCd, #")).toUpperCase() );

		console.log( JSVar(3) += JSVar(4) );
		console.log( JSVar(3) += JSVar("4") );
		console.log( JSVar("3") += JSVar(4) );
		console.log( JSVar(3) += JSVar(true) );
		console.log( JSVar("3") += JSVar(false) );
		console.log( JSVar(true) += JSVar("4") );

		console.log( JSVar(3) + JSVar(4) );
		console.log( JSVar(3) + JSVar("4") );
		console.log( JSVar("3") + JSVar(4) );
		console.log( JSVar(3) + JSVar(true) );
		console.log( JSVar("3") + JSVar(false) );
		console.log( JSVar(true) + JSVar("4") );

		console.log( JSVar("Some people when confronted with a problem think I know Ill use regular expressions Now they have two problems").match( JSRegExp( "([a-zA-Z]+)[, .]", "g" )));
		console.log( JSVar("Quick brown fox").replace( JSRegExp( "a|e|i|o|u", "g" ), JSVar("*") ));

		JSVar tested4matching = "uoaieaoau";
		console.log( tested4matching + ( JSRegExp( "[a|e|i|o|u]", "g" ).test( tested4matching ) ? "matches" : "not matches" ) );

		JSVar ttext = "abc";
		JSOwnObj badarr = makeJSArray({ 0, 0 });
		JSVar badvar = ttext.split( "" );
		badarr[0] = badvar;
		JSVar failed = badarr[0][1];
	}
};

#endif // MISC_TESTS_H
