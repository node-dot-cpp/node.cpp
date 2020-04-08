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

#ifndef STYLES_H
#define STYLES_H

#include <js_compat.h>

using namespace nodecpp::js;

class Styles_ : public nodecpp::js::JSModule
{
public:
	nodecpp::js::JSOwnObj styles = nodecpp::js::JSObject::makeJSObject();

private:
	JSOwnObj codes = nodecpp::js::JSObject::makeJSObject({ 
	  { "reset", {0, 0 } }/*,
	  { "bold", JSInitializer( {1, 22 } ) },
	  { "dim", JSInitializer( {2, 22 } ) },
	  { "italic", JSInitializer( {3, 23 } ) },
	  { "underline", JSInitializer( {4, 24 } ) },
	  { "inverse", JSInitializer( {7, 27 } ) },
	  { "hidden", JSInitializer( {8, 28 } ) },
	  { "strikethrough", JSInitializer( {9, 29 } ) },

	  { "black", JSInitializer( {30, 39 } ) },
	  { "red", JSInitializer( {31, 39 } ) },
	  { "green", JSInitializer( {32, 39 } ) },
	  { "yellow", JSInitializer( {33, 39 } ) },
	  { "blue", JSInitializer( {34, 39 } ) },
	  { "magenta", JSInitializer( {35, 39 } ) },
	  { "cyan", JSInitializer( {36, 39 } ) },
	  { "white", JSInitializer( {37, 39 } ) },
	  { "gray", JSInitializer( {90, 39 } ) },
	  { "grey", JSInitializer( {90, 39 } ) },

	  { "brightRed", JSInitializer( {91, 39 } ) },
	  { "brightGreen", JSInitializer( {92, 39 } ) },
	  { "brightYellow", JSInitializer( {93, 39 } ) },
	  { "brightBlue", JSInitializer( {94, 39 } ) },
	  { "brightMagenta", JSInitializer( {95, 39 } ) },
	  { "brightCyan", JSInitializer( {96, 39 } ) },
	  { "brightWhite", JSInitializer( {97, 39 } ) },

	  { "bgBlack", JSInitializer( {40, 49 } ) },
	  { "bgRed", JSInitializer( {41, 49 } ) },
	  { "bgGreen", JSInitializer( {42, 49 } ) },
	  { "bgYellow", JSInitializer( {43, 49 } ) },
	  { "bgBlue", JSInitializer( {44, 49 } ) },
	  { "bgMagenta", JSInitializer( {45, 49 } ) },
	  { "bgCyan", JSInitializer( {46, 49 } ) },
	  { "bgWhite", JSInitializer( {47, 49 } ) },
	  { "bgGray", JSInitializer( {100, 49 } ) },
	  { "bgGrey", JSInitializer( {100, 49 } ) },

	  { "bgBrightRed", JSInitializer( {101, 49 } ) },
	  { "bgBrightGreen", JSInitializer( {102, 49 } ) },
	  { "bgBrightYellow", JSInitializer( {103, 49 } ) },
	  { "bgBrightBlue", JSInitializer( {104, 49 } ) },
	  { "bgBrightMagenta", JSInitializer( {105, 49 } ) },
	  { "bgBrightCyan", JSInitializer( {106, 49 } ) },
	  { "bgBrightWhite", JSInitializer( {107, 49 } ) },

	  // legacy styles for colors pre v1.0.0
	  { "blackBG", JSInitializer( {40, 49 } ) },
	  { "redBG", JSInitializer( {41, 49 } ) },
	  { "greenBG", JSInitializer( {42, 49 } ) },
	  { "yellowBG", JSInitializer( {43, 49 } ) },
	  { "blueBG", JSInitializer( {44, 49 } ) },
	  { "magentaBG", JSInitializer( {45, 49 } ) },
	  { "cyanBG", JSInitializer( {46, 49 } ) },
	  { "whiteBG", JSInitializer( {47, 49} ) }*/
  
	});

public:
	Styles_()
	{
		codes.forEach([this](nodecpp::string key) {
		  auto val = codes[key];
		  styles[ key ] = JSOwnObj( std::move( nodecpp::js::JSArray::makeJSArray() ) ); // TODO: ownership
//printf( "\n======\n%s\n=======\n", styles.toString().c_str() );
		  auto style = styles[key];
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
	}
};

using Styles = JSModule2JSVar<&Styles_::styles>;

#endif // STYLES_H
