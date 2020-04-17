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

class Styles : public nodecpp::js::JSModule
{
	JSOwnObj styles = makeJSObject();

	JSOwnObj codes = makeJSObject({ 
	  { "reset", makeJSArray( {0, 0 } ) },
	  { "bold", makeJSArray( {1, 22 } ) },
	  { "dim", makeJSArray( {2, 22 } ) },
	  { "italic", makeJSArray( {3, 23 } ) },
	  { "underline", makeJSArray( {4, 24 } ) },
	  { "inverse", makeJSArray( {7, 27 } ) },
	  { "hidden", makeJSArray( {8, 28 } ) },
	  { "strikethrough", makeJSArray( {9, 29 } ) },

	  { "black", makeJSArray( {30, 39 } ) },
	  { "red", makeJSArray( {31, 39 } ) },
	  { "green", makeJSArray( {32, 39 } ) },
	  { "yellow", makeJSArray( {33, 39 } ) },
	  { "blue", makeJSArray( {34, 39 } ) },
	  { "magenta", makeJSArray( {35, 39 } ) },
	  { "cyan", makeJSArray( {36, 39 } ) },
	  { "white", makeJSArray( {37, 39 } ) },
	  { "gray", makeJSArray( {90, 39 } ) },
	  { "grey", makeJSArray( {90, 39 } ) },

	  { "brightRed", makeJSArray( {91, 39 } ) },
	  { "brightGreen", makeJSArray( {92, 39 } ) },
	  { "brightYellow", makeJSArray( {93, 39 } ) },
	  { "brightBlue", makeJSArray( {94, 39 } ) },
	  { "brightMagenta", makeJSArray( {95, 39 } ) },
	  { "brightCyan", makeJSArray( {96, 39 } ) },
	  { "brightWhite", makeJSArray( {97, 39 } ) },

	  { "bgBlack", makeJSArray( {40, 49 } ) },
	  { "bgRed", makeJSArray( {41, 49 } ) },
	  { "bgGreen", makeJSArray( {42, 49 } ) },
	  { "bgYellow", makeJSArray( {43, 49 } ) },
	  { "bgBlue", makeJSArray( {44, 49 } ) },
	  { "bgMagenta", makeJSArray( {45, 49 } ) },
	  { "bgCyan", makeJSArray( {46, 49 } ) },
	  { "bgWhite", makeJSArray( {47, 49 } ) },
	  { "bgGray", makeJSArray( {100, 49 } ) },
	  { "bgGrey", makeJSArray( {100, 49 } ) },

	  { "bgBrightRed", makeJSArray( {101, 49 } ) },
	  { "bgBrightGreen", makeJSArray( {102, 49 } ) },
	  { "bgBrightYellow", makeJSArray( {103, 49 } ) },
	  { "bgBrightBlue", makeJSArray( {104, 49 } ) },
	  { "bgBrightMagenta", makeJSArray( {105, 49 } ) },
	  { "bgBrightCyan", makeJSArray( {106, 49 } ) },
	  { "bgBrightWhite", makeJSArray( {107, 49 } ) },

	  // legacy styles for colors pre v1.0.0
	  { "blackBG", makeJSArray( {40, 49 } ) },
	  { "redBG", makeJSArray( {41, 49 } ) },
	  { "greenBG", makeJSArray( {42, 49 } ) },
	  { "yellowBG", makeJSArray( {43, 49 } ) },
	  { "blueBG", makeJSArray( {44, 49 } ) },
	  { "magentaBG", makeJSArray( {45, 49 } ) },
	  { "cyanBG", makeJSArray( {46, 49 } ) },
	  { "whiteBG", makeJSArray( {47, 49} ) }
  
	});

public:
	Styles() // initialization code is to be placed to ctor
	{
		codes.forEach([this](nodecpp::string key) {
		  auto val = codes[key];
		  styles[ key ] = JSOwnObj( std::move( makeJSArray() ) );
		  JSVar style = styles[key];
		  style[ "open" ] = nodecpp::format("\\u001b[{}m", val[0].toString() );
		  style[ "close" ] = nodecpp::format("\\u001b[{}m", val[1].toString() );
		});
	}

	JSVar exports() { return styles; } // required call returning exported var
};

#endif // STYLES_H
