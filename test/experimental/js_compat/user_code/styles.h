// JS code to be eventually converted to Node.Cpp
/////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
/*
The MIT License (MIT)

Copyright (c) Sindre Sorhus <sindresorhus@gmail.com> (sindresorhus.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

var styles = {};
module['exports'] = styles;

var codes = {
  reset: [0, 0],

  bold: [1, 22],
  dim: [2, 22],
  italic: [3, 23],
  underline: [4, 24],
  inverse: [7, 27],
  hidden: [8, 28],
  strikethrough: [9, 29],

  black: [30, 39],
  red: [31, 39],
  green: [32, 39],
  yellow: [33, 39],
  blue: [34, 39],
  magenta: [35, 39],
  cyan: [36, 39],
  white: [37, 39],
  gray: [90, 39],
  grey: [90, 39],

  brightRed: [91, 39],
  brightGreen: [92, 39],
  brightYellow: [93, 39],
  brightBlue: [94, 39],
  brightMagenta: [95, 39],
  brightCyan: [96, 39],
  brightWhite: [97, 39],

  bgBlack: [40, 49],
  bgRed: [41, 49],
  bgGreen: [42, 49],
  bgYellow: [43, 49],
  bgBlue: [44, 49],
  bgMagenta: [45, 49],
  bgCyan: [46, 49],
  bgWhite: [47, 49],
  bgGray: [100, 49],
  bgGrey: [100, 49],

  bgBrightRed: [101, 49],
  bgBrightGreen: [102, 49],
  bgBrightYellow: [103, 49],
  bgBrightBlue: [104, 49],
  bgBrightMagenta: [105, 49],
  bgBrightCyan: [106, 49],
  bgBrightWhite: [107, 49],

  // legacy styles for colors pre v1.0.0
  blackBG: [40, 49],
  redBG: [41, 49],
  greenBG: [42, 49],
  yellowBG: [43, 49],
  blueBG: [44, 49],
  magentaBG: [45, 49],
  cyanBG: [46, 49],
  whiteBG: [47, 49],

};

Object.keys(codes).forEach(function(key) {
  var val = codes[key];
  var style = styles[key] = [];
  style['open'] = '\u001b[' + val[0] + 'm';
  style.close = '\u001b[' + val[1] + 'm';
});

#endif

// CONVERTED CODE STARTS HERE
//////////////////////////////////////////////////////////////////////////////////

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

void test2()
{

auto styles = nodecpp::js::JSObject::makeJSObject();

auto codes = nodecpp::js::JSObject::makeJSObject({ 
  { "reset", nodecpp::js::JSVar::makeJSVar( {0, 0 } ) },
  { "bold", nodecpp::js::JSVar::makeJSVar( {1, 22 } ) },
  { "dim", nodecpp::js::JSVar::makeJSVar( {2, 22 } ) },
  { "italic", nodecpp::js::JSVar::makeJSVar( {3, 23 } ) },
  { "underline", nodecpp::js::JSVar::makeJSVar( {4, 24 } ) },
  { "inverse", nodecpp::js::JSVar::makeJSVar( {7, 27 } ) },
  { "hidden", nodecpp::js::JSVar::makeJSVar( {8, 28 } ) },
  { "strikethrough", nodecpp::js::JSVar::makeJSVar( {9, 29 } ) },

  { "black", nodecpp::js::JSVar::makeJSVar( {30, 39 } ) },
  { "red", nodecpp::js::JSVar::makeJSVar( {31, 39 } ) },
  { "green", nodecpp::js::JSVar::makeJSVar( {32, 39 } ) },
  { "yellow", nodecpp::js::JSVar::makeJSVar( {33, 39 } ) },
  { "blue", nodecpp::js::JSVar::makeJSVar( {34, 39 } ) },
  { "magenta", nodecpp::js::JSVar::makeJSVar( {35, 39 } ) },
  { "cyan", nodecpp::js::JSVar::makeJSVar( {36, 39 } ) },
  { "white", nodecpp::js::JSVar::makeJSVar( {37, 39 } ) },
  { "gray", nodecpp::js::JSVar::makeJSVar( {90, 39 } ) },
  { "grey", nodecpp::js::JSVar::makeJSVar( {90, 39 } ) },

  { "brightRed", nodecpp::js::JSVar::makeJSVar( {91, 39 } ) },
  { "brightGreen", nodecpp::js::JSVar::makeJSVar( {92, 39 } ) },
  { "brightYellow", nodecpp::js::JSVar::makeJSVar( {93, 39 } ) },
  { "brightBlue", nodecpp::js::JSVar::makeJSVar( {94, 39 } ) },
  { "brightMagenta", nodecpp::js::JSVar::makeJSVar( {95, 39 } ) },
  { "brightCyan", nodecpp::js::JSVar::makeJSVar( {96, 39 } ) },
  { "brightWhite", nodecpp::js::JSVar::makeJSVar( {97, 39 } ) },

  { "bgBlack", nodecpp::js::JSVar::makeJSVar( {40, 49 } ) },
  { "bgRed", nodecpp::js::JSVar::makeJSVar( {41, 49 } ) },
  { "bgGreen", nodecpp::js::JSVar::makeJSVar( {42, 49 } ) },
  { "bgYellow", nodecpp::js::JSVar::makeJSVar( {43, 49 } ) },
  { "bgBlue", nodecpp::js::JSVar::makeJSVar( {44, 49 } ) },
  { "bgMagenta", nodecpp::js::JSVar::makeJSVar( {45, 49 } ) },
  { "bgCyan", nodecpp::js::JSVar::makeJSVar( {46, 49 } ) },
  { "bgWhite", nodecpp::js::JSVar::makeJSVar( {47, 49 } ) },
  { "bgGray", nodecpp::js::JSVar::makeJSVar( {100, 49 } ) },
  { "bgGrey", nodecpp::js::JSVar::makeJSVar( {100, 49 } ) },

  { "bgBrightRed", nodecpp::js::JSVar::makeJSVar( {101, 49 } ) },
  { "bgBrightGreen", nodecpp::js::JSVar::makeJSVar( {102, 49 } ) },
  { "bgBrightYellow", nodecpp::js::JSVar::makeJSVar( {103, 49 } ) },
  { "bgBrightBlue", nodecpp::js::JSVar::makeJSVar( {104, 49 } ) },
  { "bgBrightMagenta", nodecpp::js::JSVar::makeJSVar( {105, 49 } ) },
  { "bgBrightCyan", nodecpp::js::JSVar::makeJSVar( {106, 49 } ) },
  { "bgBrightWhite", nodecpp::js::JSVar::makeJSVar( {107, 49 } ) },

  // legacy styles for colors pre v1.0.0
  { "blackBG", nodecpp::js::JSVar::makeJSVar( {40, 49 } ) },
  { "redBG", nodecpp::js::JSVar::makeJSVar( {41, 49 } ) },
  { "greenBG", nodecpp::js::JSVar::makeJSVar( {42, 49 } ) },
  { "yellowBG", nodecpp::js::JSVar::makeJSVar( {43, 49 } ) },
  { "blueBG", nodecpp::js::JSVar::makeJSVar( {44, 49 } ) },
  { "magentaBG", nodecpp::js::JSVar::makeJSVar( {45, 49 } ) },
  { "cyanBG", nodecpp::js::JSVar::makeJSVar( {46, 49 } ) },
  { "whiteBG", nodecpp::js::JSVar::makeJSVar( {47, 49} ) }
  
});

codes->forEach([&codes, &styles](nodecpp::string key) {
  auto val = (*codes)[key];
  styles->add( key, std::move( nodecpp::js::JSVar::makeJSVar( std::move( nodecpp::js::JSArray::makeJSArray() ) ) ) );
  auto style = (*styles)[key];
//  style->add( "open", nodecpp::js::JSVar::makeJSVar( nodecpp::format("\\u001b[{}m", (*val)[0]->toString() ) ) );
//  style->add( "close", nodecpp::js::JSVar::makeJSVar( nodecpp::format("\\u001b[{}m", (*val)[0]->toString() ) ) );
  style->add( "open", std::move( nodecpp::js::JSVar::makeJSVar( nodecpp::format("\\u001b[{}m", (*val)[0]->toString() ) ) ) );
  style->add( "close", std::move( nodecpp::js::JSVar::makeJSVar( nodecpp::format("\\u001b[{}m", (*val)[1]->toString() ) ) ) );
});

printf( "%s\n", styles->toString().c_str() );

}

class Styles
{
	template<class T>
	nodecpp::safememory::soft_ptr<nodecpp::js::JSVar> friend nodecpp::js::require<Styles>();

//	nodecpp::js::JSVar exports;

	static nodecpp::safememory::owning_ptr<nodecpp::js::JSVar> getExported() {

auto styles = nodecpp::js::JSObject::makeJSObject();

auto codes = nodecpp::js::JSObject::makeJSObject({ 
  { "reset", nodecpp::js::JSVar::makeJSVar( {0, 0 } ) },
  { "bold", nodecpp::js::JSVar::makeJSVar( {1, 22 } ) },
  { "dim", nodecpp::js::JSVar::makeJSVar( {2, 22 } ) },
  { "italic", nodecpp::js::JSVar::makeJSVar( {3, 23 } ) },
  { "underline", nodecpp::js::JSVar::makeJSVar( {4, 24 } ) },
  { "inverse", nodecpp::js::JSVar::makeJSVar( {7, 27 } ) },
  { "hidden", nodecpp::js::JSVar::makeJSVar( {8, 28 } ) },
  { "strikethrough", nodecpp::js::JSVar::makeJSVar( {9, 29 } ) },

  { "black", nodecpp::js::JSVar::makeJSVar( {30, 39 } ) },
  { "red", nodecpp::js::JSVar::makeJSVar( {31, 39 } ) },
  { "green", nodecpp::js::JSVar::makeJSVar( {32, 39 } ) },
  { "yellow", nodecpp::js::JSVar::makeJSVar( {33, 39 } ) },
  { "blue", nodecpp::js::JSVar::makeJSVar( {34, 39 } ) },
  { "magenta", nodecpp::js::JSVar::makeJSVar( {35, 39 } ) },
  { "cyan", nodecpp::js::JSVar::makeJSVar( {36, 39 } ) },
  { "white", nodecpp::js::JSVar::makeJSVar( {37, 39 } ) },
  { "gray", nodecpp::js::JSVar::makeJSVar( {90, 39 } ) },
  { "grey", nodecpp::js::JSVar::makeJSVar( {90, 39 } ) },

  { "brightRed", nodecpp::js::JSVar::makeJSVar( {91, 39 } ) },
  { "brightGreen", nodecpp::js::JSVar::makeJSVar( {92, 39 } ) },
  { "brightYellow", nodecpp::js::JSVar::makeJSVar( {93, 39 } ) },
  { "brightBlue", nodecpp::js::JSVar::makeJSVar( {94, 39 } ) },
  { "brightMagenta", nodecpp::js::JSVar::makeJSVar( {95, 39 } ) },
  { "brightCyan", nodecpp::js::JSVar::makeJSVar( {96, 39 } ) },
  { "brightWhite", nodecpp::js::JSVar::makeJSVar( {97, 39 } ) },

  { "bgBlack", nodecpp::js::JSVar::makeJSVar( {40, 49 } ) },
  { "bgRed", nodecpp::js::JSVar::makeJSVar( {41, 49 } ) },
  { "bgGreen", nodecpp::js::JSVar::makeJSVar( {42, 49 } ) },
  { "bgYellow", nodecpp::js::JSVar::makeJSVar( {43, 49 } ) },
  { "bgBlue", nodecpp::js::JSVar::makeJSVar( {44, 49 } ) },
  { "bgMagenta", nodecpp::js::JSVar::makeJSVar( {45, 49 } ) },
  { "bgCyan", nodecpp::js::JSVar::makeJSVar( {46, 49 } ) },
  { "bgWhite", nodecpp::js::JSVar::makeJSVar( {47, 49 } ) },
  { "bgGray", nodecpp::js::JSVar::makeJSVar( {100, 49 } ) },
  { "bgGrey", nodecpp::js::JSVar::makeJSVar( {100, 49 } ) },

  { "bgBrightRed", nodecpp::js::JSVar::makeJSVar( {101, 49 } ) },
  { "bgBrightGreen", nodecpp::js::JSVar::makeJSVar( {102, 49 } ) },
  { "bgBrightYellow", nodecpp::js::JSVar::makeJSVar( {103, 49 } ) },
  { "bgBrightBlue", nodecpp::js::JSVar::makeJSVar( {104, 49 } ) },
  { "bgBrightMagenta", nodecpp::js::JSVar::makeJSVar( {105, 49 } ) },
  { "bgBrightCyan", nodecpp::js::JSVar::makeJSVar( {106, 49 } ) },
  { "bgBrightWhite", nodecpp::js::JSVar::makeJSVar( {107, 49 } ) },

  // legacy styles for colors pre v1.0.0
  { "blackBG", nodecpp::js::JSVar::makeJSVar( {40, 49 } ) },
  { "redBG", nodecpp::js::JSVar::makeJSVar( {41, 49 } ) },
  { "greenBG", nodecpp::js::JSVar::makeJSVar( {42, 49 } ) },
  { "yellowBG", nodecpp::js::JSVar::makeJSVar( {43, 49 } ) },
  { "blueBG", nodecpp::js::JSVar::makeJSVar( {44, 49 } ) },
  { "magentaBG", nodecpp::js::JSVar::makeJSVar( {45, 49 } ) },
  { "cyanBG", nodecpp::js::JSVar::makeJSVar( {46, 49 } ) },
  { "whiteBG", nodecpp::js::JSVar::makeJSVar( {47, 49} ) }
  
});

codes->forEach([&codes, &styles](nodecpp::string key) {
  auto val = (*codes)[key];
  styles->add( key, std::move( nodecpp::js::JSVar::makeJSVar( std::move( nodecpp::js::JSArray::makeJSArray() ) ) ) );
  auto style = (*styles)[key];
//  style->add( "open", nodecpp::js::JSVar::makeJSVar( nodecpp::format("\\u001b[{}m", (*val)[0]->toString() ) ) );
//  style->add( "close", nodecpp::js::JSVar::makeJSVar( nodecpp::format("\\u001b[{}m", (*val)[0]->toString() ) ) );
  style->add( "open", std::move( nodecpp::js::JSVar::makeJSVar( nodecpp::format("\\u001b[{}m", (*val)[0]->toString() ) ) ) );
  style->add( "close", std::move( nodecpp::js::JSVar::makeJSVar( nodecpp::format("\\u001b[{}m", (*val)[1]->toString() ) ) ) );
});

//printf( "%s\n", styles->toString().c_str() );

//		return nodecpp::safememory::owning_ptr<nodecpp::js::JSVar>();
return nodecpp::js::JSVar::makeJSVar( std::move( styles ) );
	}
public:
	Styles()
	{
	}

};

class Styles2
{
public:
	nodecpp::safememory::owning_ptr<nodecpp::js::JSVar> styles = nodecpp::js::JSVar::makeJSVar();

private:
nodecpp::safememory::owning_ptr<nodecpp::js::JSObject> codes = nodecpp::js::JSObject::makeJSObject({ 
  { "reset", nodecpp::js::JSVar::makeJSVar( {0, 0 } ) },
  { "bold", nodecpp::js::JSVar::makeJSVar( {1, 22 } ) },
  { "dim", nodecpp::js::JSVar::makeJSVar( {2, 22 } ) },
  { "italic", nodecpp::js::JSVar::makeJSVar( {3, 23 } ) },
  { "underline", nodecpp::js::JSVar::makeJSVar( {4, 24 } ) },
  { "inverse", nodecpp::js::JSVar::makeJSVar( {7, 27 } ) },
  { "hidden", nodecpp::js::JSVar::makeJSVar( {8, 28 } ) },
  { "strikethrough", nodecpp::js::JSVar::makeJSVar( {9, 29 } ) },

  { "black", nodecpp::js::JSVar::makeJSVar( {30, 39 } ) },
  { "red", nodecpp::js::JSVar::makeJSVar( {31, 39 } ) },
  { "green", nodecpp::js::JSVar::makeJSVar( {32, 39 } ) },
  { "yellow", nodecpp::js::JSVar::makeJSVar( {33, 39 } ) },
  { "blue", nodecpp::js::JSVar::makeJSVar( {34, 39 } ) },
  { "magenta", nodecpp::js::JSVar::makeJSVar( {35, 39 } ) },
  { "cyan", nodecpp::js::JSVar::makeJSVar( {36, 39 } ) },
  { "white", nodecpp::js::JSVar::makeJSVar( {37, 39 } ) },
  { "gray", nodecpp::js::JSVar::makeJSVar( {90, 39 } ) },
  { "grey", nodecpp::js::JSVar::makeJSVar( {90, 39 } ) },

  { "brightRed", nodecpp::js::JSVar::makeJSVar( {91, 39 } ) },
  { "brightGreen", nodecpp::js::JSVar::makeJSVar( {92, 39 } ) },
  { "brightYellow", nodecpp::js::JSVar::makeJSVar( {93, 39 } ) },
  { "brightBlue", nodecpp::js::JSVar::makeJSVar( {94, 39 } ) },
  { "brightMagenta", nodecpp::js::JSVar::makeJSVar( {95, 39 } ) },
  { "brightCyan", nodecpp::js::JSVar::makeJSVar( {96, 39 } ) },
  { "brightWhite", nodecpp::js::JSVar::makeJSVar( {97, 39 } ) },

  { "bgBlack", nodecpp::js::JSVar::makeJSVar( {40, 49 } ) },
  { "bgRed", nodecpp::js::JSVar::makeJSVar( {41, 49 } ) },
  { "bgGreen", nodecpp::js::JSVar::makeJSVar( {42, 49 } ) },
  { "bgYellow", nodecpp::js::JSVar::makeJSVar( {43, 49 } ) },
  { "bgBlue", nodecpp::js::JSVar::makeJSVar( {44, 49 } ) },
  { "bgMagenta", nodecpp::js::JSVar::makeJSVar( {45, 49 } ) },
  { "bgCyan", nodecpp::js::JSVar::makeJSVar( {46, 49 } ) },
  { "bgWhite", nodecpp::js::JSVar::makeJSVar( {47, 49 } ) },
  { "bgGray", nodecpp::js::JSVar::makeJSVar( {100, 49 } ) },
  { "bgGrey", nodecpp::js::JSVar::makeJSVar( {100, 49 } ) },

  { "bgBrightRed", nodecpp::js::JSVar::makeJSVar( {101, 49 } ) },
  { "bgBrightGreen", nodecpp::js::JSVar::makeJSVar( {102, 49 } ) },
  { "bgBrightYellow", nodecpp::js::JSVar::makeJSVar( {103, 49 } ) },
  { "bgBrightBlue", nodecpp::js::JSVar::makeJSVar( {104, 49 } ) },
  { "bgBrightMagenta", nodecpp::js::JSVar::makeJSVar( {105, 49 } ) },
  { "bgBrightCyan", nodecpp::js::JSVar::makeJSVar( {106, 49 } ) },
  { "bgBrightWhite", nodecpp::js::JSVar::makeJSVar( {107, 49 } ) },

  // legacy styles for colors pre v1.0.0
  { "blackBG", nodecpp::js::JSVar::makeJSVar( {40, 49 } ) },
  { "redBG", nodecpp::js::JSVar::makeJSVar( {41, 49 } ) },
  { "greenBG", nodecpp::js::JSVar::makeJSVar( {42, 49 } ) },
  { "yellowBG", nodecpp::js::JSVar::makeJSVar( {43, 49 } ) },
  { "blueBG", nodecpp::js::JSVar::makeJSVar( {44, 49 } ) },
  { "magentaBG", nodecpp::js::JSVar::makeJSVar( {45, 49 } ) },
  { "cyanBG", nodecpp::js::JSVar::makeJSVar( {46, 49 } ) },
  { "whiteBG", nodecpp::js::JSVar::makeJSVar( {47, 49} ) }
  
});


public:
	Styles2()
	{
		codes->forEach([this](nodecpp::string key) {
		  auto val = (*codes)[key];
		  styles->add( key, std::move( nodecpp::js::JSVar::makeJSVar( std::move( nodecpp::js::JSArray::makeJSArray() ) ) ) );
		  auto style = (*styles)[key];
		  style->add( "open", std::move( nodecpp::js::JSVar::makeJSVar( nodecpp::format("\\u001b[{}m", (*val)[0]->toString() ) ) ) );
		  style->add( "close", std::move( nodecpp::js::JSVar::makeJSVar( nodecpp::format("\\u001b[{}m", (*val)[1]->toString() ) ) ) );
		});
	}
};

class Styles3 : public nodecpp::js::JSModule
{
public:
	nodecpp::safememory::owning_ptr<nodecpp::js::JSVar> styles = nodecpp::js::JSVar::makeJSVar();

private:
	nodecpp::safememory::owning_ptr<nodecpp::js::JSObject> codes = nodecpp::js::JSObject::makeJSObject({ 
	  { "reset", nodecpp::js::JSVar::makeJSVar( {0, 0 } ) },
	  { "bold", nodecpp::js::JSVar::makeJSVar( {1, 22 } ) },
	  { "dim", nodecpp::js::JSVar::makeJSVar( {2, 22 } ) },
	  { "italic", nodecpp::js::JSVar::makeJSVar( {3, 23 } ) },
	  { "underline", nodecpp::js::JSVar::makeJSVar( {4, 24 } ) },
	  { "inverse", nodecpp::js::JSVar::makeJSVar( {7, 27 } ) },
	  { "hidden", nodecpp::js::JSVar::makeJSVar( {8, 28 } ) },
	  { "strikethrough", nodecpp::js::JSVar::makeJSVar( {9, 29 } ) },

	  { "black", nodecpp::js::JSVar::makeJSVar( {30, 39 } ) },
	  { "red", nodecpp::js::JSVar::makeJSVar( {31, 39 } ) },
	  { "green", nodecpp::js::JSVar::makeJSVar( {32, 39 } ) },
	  { "yellow", nodecpp::js::JSVar::makeJSVar( {33, 39 } ) },
	  { "blue", nodecpp::js::JSVar::makeJSVar( {34, 39 } ) },
	  { "magenta", nodecpp::js::JSVar::makeJSVar( {35, 39 } ) },
	  { "cyan", nodecpp::js::JSVar::makeJSVar( {36, 39 } ) },
	  { "white", nodecpp::js::JSVar::makeJSVar( {37, 39 } ) },
	  { "gray", nodecpp::js::JSVar::makeJSVar( {90, 39 } ) },
	  { "grey", nodecpp::js::JSVar::makeJSVar( {90, 39 } ) },

	  { "brightRed", nodecpp::js::JSVar::makeJSVar( {91, 39 } ) },
	  { "brightGreen", nodecpp::js::JSVar::makeJSVar( {92, 39 } ) },
	  { "brightYellow", nodecpp::js::JSVar::makeJSVar( {93, 39 } ) },
	  { "brightBlue", nodecpp::js::JSVar::makeJSVar( {94, 39 } ) },
	  { "brightMagenta", nodecpp::js::JSVar::makeJSVar( {95, 39 } ) },
	  { "brightCyan", nodecpp::js::JSVar::makeJSVar( {96, 39 } ) },
	  { "brightWhite", nodecpp::js::JSVar::makeJSVar( {97, 39 } ) },

	  { "bgBlack", nodecpp::js::JSVar::makeJSVar( {40, 49 } ) },
	  { "bgRed", nodecpp::js::JSVar::makeJSVar( {41, 49 } ) },
	  { "bgGreen", nodecpp::js::JSVar::makeJSVar( {42, 49 } ) },
	  { "bgYellow", nodecpp::js::JSVar::makeJSVar( {43, 49 } ) },
	  { "bgBlue", nodecpp::js::JSVar::makeJSVar( {44, 49 } ) },
	  { "bgMagenta", nodecpp::js::JSVar::makeJSVar( {45, 49 } ) },
	  { "bgCyan", nodecpp::js::JSVar::makeJSVar( {46, 49 } ) },
	  { "bgWhite", nodecpp::js::JSVar::makeJSVar( {47, 49 } ) },
	  { "bgGray", nodecpp::js::JSVar::makeJSVar( {100, 49 } ) },
	  { "bgGrey", nodecpp::js::JSVar::makeJSVar( {100, 49 } ) },

	  { "bgBrightRed", nodecpp::js::JSVar::makeJSVar( {101, 49 } ) },
	  { "bgBrightGreen", nodecpp::js::JSVar::makeJSVar( {102, 49 } ) },
	  { "bgBrightYellow", nodecpp::js::JSVar::makeJSVar( {103, 49 } ) },
	  { "bgBrightBlue", nodecpp::js::JSVar::makeJSVar( {104, 49 } ) },
	  { "bgBrightMagenta", nodecpp::js::JSVar::makeJSVar( {105, 49 } ) },
	  { "bgBrightCyan", nodecpp::js::JSVar::makeJSVar( {106, 49 } ) },
	  { "bgBrightWhite", nodecpp::js::JSVar::makeJSVar( {107, 49 } ) },

	  // legacy styles for colors pre v1.0.0
	  { "blackBG", nodecpp::js::JSVar::makeJSVar( {40, 49 } ) },
	  { "redBG", nodecpp::js::JSVar::makeJSVar( {41, 49 } ) },
	  { "greenBG", nodecpp::js::JSVar::makeJSVar( {42, 49 } ) },
	  { "yellowBG", nodecpp::js::JSVar::makeJSVar( {43, 49 } ) },
	  { "blueBG", nodecpp::js::JSVar::makeJSVar( {44, 49 } ) },
	  { "magentaBG", nodecpp::js::JSVar::makeJSVar( {45, 49 } ) },
	  { "cyanBG", nodecpp::js::JSVar::makeJSVar( {46, 49 } ) },
	  { "whiteBG", nodecpp::js::JSVar::makeJSVar( {47, 49} ) }
  
	});

public:
	Styles3()
	{
		codes->forEach([this](nodecpp::string key) {
		  auto val = (*codes)[key];
		  styles->add( key, std::move( nodecpp::js::JSVar::makeJSVar( std::move( nodecpp::js::JSArray::makeJSArray() ) ) ) );
		  auto style = (*styles)[key];
		  style->add( "open", std::move( nodecpp::js::JSVar::makeJSVar( nodecpp::format("\\u001b[{}m", (*val)[0]->toString() ) ) ) );
		  style->add( "close", std::move( nodecpp::js::JSVar::makeJSVar( nodecpp::format("\\u001b[{}m", (*val)[1]->toString() ) ) ) );
		});
	}
};


#endif // STYLES_H
