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
	nodecpp::js::JSVar styles;

private:
	nodecpp::safememory::owning_ptr<nodecpp::js::JSObject> codes = nodecpp::js::JSObject::makeJSObject({ 
	  { "reset", JSVar( {0, 0 } ) },
	  { "bold", JSVar( {1, 22 } ) },
	  { "dim", JSVar( {2, 22 } ) },
	  { "italic", JSVar( {3, 23 } ) },
	  { "underline", JSVar( {4, 24 } ) },
	  { "inverse", JSVar( {7, 27 } ) },
	  { "hidden", JSVar( {8, 28 } ) },
	  { "strikethrough", JSVar( {9, 29 } ) },

	  { "black", JSVar( {30, 39 } ) },
	  { "red", JSVar( {31, 39 } ) },
	  { "green", JSVar( {32, 39 } ) },
	  { "yellow", JSVar( {33, 39 } ) },
	  { "blue", JSVar( {34, 39 } ) },
	  { "magenta", JSVar( {35, 39 } ) },
	  { "cyan", JSVar( {36, 39 } ) },
	  { "white", JSVar( {37, 39 } ) },
	  { "gray", JSVar( {90, 39 } ) },
	  { "grey", JSVar( {90, 39 } ) },

	  { "brightRed", JSVar( {91, 39 } ) },
	  { "brightGreen", JSVar( {92, 39 } ) },
	  { "brightYellow", JSVar( {93, 39 } ) },
	  { "brightBlue", JSVar( {94, 39 } ) },
	  { "brightMagenta", JSVar( {95, 39 } ) },
	  { "brightCyan", JSVar( {96, 39 } ) },
	  { "brightWhite", JSVar( {97, 39 } ) },

	  { "bgBlack", JSVar( {40, 49 } ) },
	  { "bgRed", JSVar( {41, 49 } ) },
	  { "bgGreen", JSVar( {42, 49 } ) },
	  { "bgYellow", JSVar( {43, 49 } ) },
	  { "bgBlue", JSVar( {44, 49 } ) },
	  { "bgMagenta", JSVar( {45, 49 } ) },
	  { "bgCyan", JSVar( {46, 49 } ) },
	  { "bgWhite", JSVar( {47, 49 } ) },
	  { "bgGray", JSVar( {100, 49 } ) },
	  { "bgGrey", JSVar( {100, 49 } ) },

	  { "bgBrightRed", JSVar( {101, 49 } ) },
	  { "bgBrightGreen", JSVar( {102, 49 } ) },
	  { "bgBrightYellow", JSVar( {103, 49 } ) },
	  { "bgBrightBlue", JSVar( {104, 49 } ) },
	  { "bgBrightMagenta", JSVar( {105, 49 } ) },
	  { "bgBrightCyan", JSVar( {106, 49 } ) },
	  { "bgBrightWhite", JSVar( {107, 49 } ) },

	  // legacy styles for colors pre v1.0.0
	  { "blackBG", JSVar( {40, 49 } ) },
	  { "redBG", JSVar( {41, 49 } ) },
	  { "greenBG", JSVar( {42, 49 } ) },
	  { "yellowBG", JSVar( {43, 49 } ) },
	  { "blueBG", JSVar( {44, 49 } ) },
	  { "magentaBG", JSVar( {45, 49 } ) },
	  { "cyanBG", JSVar( {46, 49 } ) },
	  { "whiteBG", JSVar( {47, 49} ) }
  
	});

public:
	Styles_()
	{
		codes->forEach([this](nodecpp::string key) {
		  auto val = (*codes)[key];
		  styles[ key ] = JSInitializer( std::move( nodecpp::js::JSArray::makeJSArray() ) ); // TODO: ownership
		  auto style = styles[key];
		  style[ "open" ] = JSInitializer( nodecpp::format("\\u001b[{}m", val[0].toString() ) );
		  style[ "close" ] = JSInitializer( nodecpp::format("\\u001b[{}m", val[1].toString() ) );
		});
	}
};

using Styles = JSModule2JSVar<&Styles_::styles>;

#endif // STYLES_H
