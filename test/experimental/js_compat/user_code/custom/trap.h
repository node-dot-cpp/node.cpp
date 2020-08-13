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

#ifndef TRAP_H
#define TRAP_H

#include <nodecpp/js_compat.h>

using namespace nodecpp::js;

class Trap : public nodecpp::js::JSModule
{
private:
    JSOwnObj trap_ = makeJSObject({
        {"a",makeJSArray({"\u0040", "\u0104", "\u023a", "\u0245", "\u0394", "\u039b", "\u0414"})},
        {"b",makeJSArray({"\u00df", "\u0181", "\u0243", "\u026e", "\u03b2", "\u0e3f"})},
        {"c",makeJSArray({"\u00a9", "\u023b", "\u03fe"})},
        {"d",makeJSArray({"\u00d0", "\u018a", "\u0500", "\u0501", "\u0502", "\u0503"})},
        {"e",makeJSArray({"\u00cb", "\u0115", "\u018e", "\u0258", "\u03a3", "\u03be", "\u04bc","\u0a6c"})},
        {"f",makeJSArray({"\u04fa"})},
        {"g",makeJSArray({"\u0262"})},
        {"h",makeJSArray({"\u0126", "\u0195", "\u04a2", "\u04ba", "\u04c7", "\u050a"})},
        {"i",makeJSArray({"\u0f0f"})},
        {"j",makeJSArray({"\u0134"})},
        {"k",makeJSArray({"\u0138", "\u04a0", "\u04c3", "\u051e"})},
        {"l",makeJSArray({"\u0139"})},
        {"m",makeJSArray({"\u028d", "\u04cd", "\u04ce", "\u0520", "\u0521", "\u0d69"})},
        {"n",makeJSArray({"\u00d1", "\u014b", "\u019d", "\u0376", "\u03a0", "\u048a"})},
        {"o",makeJSArray({"\u00d8", "\u00f5", "\u00f8", "\u01fe", "\u0298", "\u047a", "\u05dd","\u06dd", "\u0e4f"})},
        {"p",makeJSArray({"\u01f7", "\u048e"})},
        {"q",makeJSArray({"\u09cd"})},
        {"r",makeJSArray({"\u00ae", "\u01a6", "\u0210", "\u024c", "\u0280", "\u042f"})},
        {"s",makeJSArray({"\u00a7", "\u03de", "\u03df", "\u03e8"})},
        {"t",makeJSArray({"\u0141", "\u0166", "\u0373"})},
        {"u",makeJSArray({"\u01b1", "\u054d"})},
        {"v",makeJSArray({"\u05d8"})},
        {"w",makeJSArray({"\u0428", "\u0460", "\u047c", "\u0d70"})},
        {"x",makeJSArray({"\u04b2", "\u04fe", "\u04fc", "\u04fd"})},
        {"y",makeJSArray({"\u00a5", "\u04b0", "\u04cb"})},
        {"z",makeJSArray({"\u01b5", "\u0240"})}
        });
public:
	JSVar exports() {
		JSVar trap = trap_;
		return 
			JSVar([trap](JSVar text, JSVar options) {
			    JSVar result = "";
			    text = text || JSVar("Run the trap drop the bass");
			    JSOwnObj textChars = text.split("");
			    text = textChars;
			    text.forEach([trap, &result, text](JSVar c) {
				    c = c.toLowerCase();
				    JSOwnObj space = makeJSArray({ " " });
				    JSVar chars = trap[c] || space; // var chars = trap[c] || [' '];
				    JSVar rand = JSMath::floor(JSMath::random() * chars.length());
				    if (typeOf(trap[c]) != "undefined") {
					    result += trap[c][rand];
				    }
				    else {
					    result += c;
				    }
				});
			    return result;
		    });
    }
};
#endif // TRAP_H

