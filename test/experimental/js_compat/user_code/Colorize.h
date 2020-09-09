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

#ifndef COLORIZE_H
#define COLORIZE_H

#include <nodecpp/js_compat.h>

using namespace nodecpp::js;

class Colorize : public nodecpp::js::JSModule
{
	JSOwnObj styles = makeJSObject();

	JSOwnObj codes_ = makeJSObject({
	  { "reset", makeJSArray({0, 0 }) },
	  { "bold", makeJSArray({1, 22 }) },
	  { "dim", makeJSArray({2, 22 }) },
	  { "italic", makeJSArray({3, 23 }) },
	  { "underline", makeJSArray({4, 24 }) },
	  { "inverse", makeJSArray({7, 27 }) },
	  { "hidden", makeJSArray({8, 28 }) },
	  { "strikethrough", makeJSArray({9, 29 }) },

	  { "black", makeJSArray({30, 39 }) },
	  { "red", makeJSArray({31, 39 }) },
	  { "green", makeJSArray({32, 39 }) },
	  { "yellow", makeJSArray({33, 39 }) },
	  { "blue", makeJSArray({34, 39 }) },
	  { "magenta", makeJSArray({35, 39 }) },
	  { "cyan", makeJSArray({36, 39 }) },
	  { "white", makeJSArray({37, 39 }) },
	  { "gray", makeJSArray({90, 39 }) },
	  { "grey", makeJSArray({90, 39 }) },

	  { "brightRed", makeJSArray({91, 39 }) },
	  { "brightGreen", makeJSArray({92, 39 }) },
	  { "brightYellow", makeJSArray({93, 39 }) },
	  { "brightBlue", makeJSArray({94, 39 }) },
	  { "brightMagenta", makeJSArray({95, 39 }) },
	  { "brightCyan", makeJSArray({96, 39 }) },
	  { "brightWhite", makeJSArray({97, 39 }) },

	  { "bgBlack", makeJSArray({40, 49 }) },
	  { "bgRed", makeJSArray({41, 49 }) },
	  { "bgGreen", makeJSArray({42, 49 }) },
	  { "bgYellow", makeJSArray({43, 49 }) },
	  { "bgBlue", makeJSArray({44, 49 }) },
	  { "bgMagenta", makeJSArray({45, 49 }) },
	  { "bgCyan", makeJSArray({46, 49 }) },
	  { "bgWhite", makeJSArray({47, 49 }) },
	  { "bgGray", makeJSArray({100, 49 }) },
	  { "bgGrey", makeJSArray({100, 49 }) },

	  { "bgBrightRed", makeJSArray({101, 49 }) },
	  { "bgBrightGreen", makeJSArray({102, 49 }) },
	  { "bgBrightYellow", makeJSArray({103, 49 }) },
	  { "bgBrightBlue", makeJSArray({104, 49 }) },
	  { "bgBrightMagenta", makeJSArray({105, 49 }) },
	  { "bgBrightCyan", makeJSArray({106, 49 }) },
	  { "bgBrightWhite", makeJSArray({107, 49 }) },

	  // legacy styles for colors pre v1.0.0
	  { "blackBG", makeJSArray({40, 49 }) },
	  { "redBG", makeJSArray({41, 49 }) },
	  { "greenBG", makeJSArray({42, 49 }) },
	  { "yellowBG", makeJSArray({43, 49 }) },
	  { "blueBG", makeJSArray({44, 49 }) },
	  { "magentaBG", makeJSArray({45, 49 }) },
	  { "cyanBG", makeJSArray({46, 49 }) },
	  { "whiteBG", makeJSArray({47, 49}) }

		});

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

	JSOwnObj rainbowColors_ = makeJSArray({ "red", "yellow", "green", "blue", "magenta" });

	JSOwnObj available_ = makeJSArray({ "underline", "inverse", "grey", "yellow", "red", "green","blue",
								  "white", "cyan", "magenta", "brightYellow", "brightRed","brightGreen",
								  "brightBlue", "brightWhite", "brightCyan", "brightMagenta" });

	JSOwnObj setTheme_ = makeJSObject({
		{"silly",makeJSArray({"rainbow"})},
		{"input",makeJSArray({"grey"})},
		{"verbose",makeJSArray({"cyan"})},
		{"prompt",makeJSArray({"grey"})},
		{"info",makeJSArray({"green"})},
		{"data",makeJSArray({"grey"})},
		{"help",makeJSArray({"cyan"})},
		{"warn",makeJSArray({"yellow"})},
		{"debug",makeJSArray({"blue"})},
		{"error",makeJSArray({"red"})},
		{"custom",makeJSArray({"whiteBG"})},
		});
public:
	nodecpp::string stylize(JSVar str, JSVar color) {
		JSVar codes = codes_;
		JSVar  code = codes[color];
		return nodecpp::format("\u001b[{}m{}\u001b[{}m", code[0].toString(), str.toString(), code[1].toString());
	}
	nodecpp::string trap(JSVar str) {
		JSVar trap = trap_;
		JSVar result = "";
		str = str || JSVar("Run the trap drop the bass");
		JSOwnObj textChars = str.split("");
		str = textChars;
		str.forEach([trap, &result, str](JSVar c) {
			c = c.toLowerCase();
			JSOwnObj space = makeJSArray({ " " });
			JSVar chars = trap[c] || space; 
			JSVar rand = JSMath::floor(JSMath::random() * chars.length());
			if (typeOf(trap[c]) != "undefined") {
				result += trap[c][rand];
			}
			else {
				result += c;
			}
			});
		return result;
	}
	nodecpp::string zebra(JSVar str) {
		JSVar mappedStr = "";
		JSOwnObj textChars = str.split("");
		str = textChars;
		for (int i = 0; i < str.length(); i++) {
			if (i % 2 == 0) {
				mappedStr += str[i];
			}
			else {
				mappedStr += stylize(str[i], "inverse");
			}
		};
		return mappedStr;
	}
	nodecpp::string random(JSVar str) {
		JSVar available = available_;
		JSVar mappedStr = "";
		JSOwnObj textChars = str.split("");
		str = textChars;
		for (int i = 0; i < str.length(); i++) {
			JSVar letter = str[i];
			letter.isStrictlyTheSame(" ") ? mappedStr += letter : mappedStr += stylize(letter, available[JSMath::random() * (available.length() - 2)]);
		};
		return mappedStr;
	}

	
	nodecpp::string rainbow(JSVar str) {
		JSVar rainbowColors = rainbowColors_;
		JSVar mappedStr = "";
		JSOwnObj textChars = str.split("");
		str = textChars;
		for (int i = 0; i < str.length(); i++) {
			JSVar letter = str[i];
			letter.isStrictlyTheSame(" ") ? mappedStr += letter : mappedStr += stylize(letter, rainbowColors[i++ % (int)(rainbowColors.length())]);// TODO:
		};
		return mappedStr;
	}

	nodecpp::string america(JSVar str) {
		JSVar available = available_;
		JSVar mappedStr = "";
		JSOwnObj textChars = str.split("");
		str = textChars;
		for (int i = 0; i < str.length(); i++) {
			JSVar letter = str[i];
			if (letter.isStrictlyTheSame(" ")) {
				mappedStr += str[i];
			}
			else {
				switch (i % 3) // JS: '  switch (i%3)  ' where i is var. TODO: consider '  switch ( (size_t)((i % 3).toNumber()) )  ', which is closer to the original
				{
				case 0:
					 mappedStr += stylize(letter, "red");
					 break;
				case 1:
					 mappedStr += stylize(letter, "white");
					 break;
				case 2:
					 mappedStr += stylize(letter, "blue");
					 break;

				}
			};
		};
		return mappedStr;
	}


	nodecpp::string applyTheme(JSVar str, JSVar theme ) {
		JSVar setTheme = setTheme_;
		JSVar theme_ = setTheme[theme];
		for (int i = 0; i < theme_.length(); i++) {
			str = stylize(str, theme_[i]);
		};
		return str;
	}

	Colorize(){}
};

#endif // COLORIZE_H
