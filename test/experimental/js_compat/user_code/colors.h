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

#ifndef COLORS_H
#define COLORS_H

#include <nodecpp/js_compat.h>
#include "styles.h"

using namespace nodecpp::js;

class Colors : public JSModule
{
public:
	JSVar colors;

private:
	JSVar ansiStyles;


public:
	Colors()
	{
		ansiStyles = require<Styles>();
	}

	void enable()
	{
		colors["enbled"] = true;
	}

	void disable()
	{
		colors["enbled"] = false;
	}

#if 1 // temporarily disabled
#endif // 0
	JSVar stylize( JSVar str, JSVar style)
	{
		if (!colors["enbled"])
			return JSVar( str.toString() );

		JSVar styleMap = ansiStyles[style];

		// Stylize should work for non-ANSI styles, too
		if( !styleMap && style.in( colors ) )
		{
			// Style maps like trap operate as functions on strings;
			// they don't have properties like open or close.
//			return (*colors)[*style](*str);
		}

		return styleMap["open"].toString() + str.toString() + styleMap["close"].toString();
	}

	// fake temporary call for testing purposes only; TODO remove when no longer needed ASAP
	nodecpp::string toString() { return "Colors class"; }
};

#endif // COLORS_H
