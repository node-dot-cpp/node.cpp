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

#ifndef SETMAP_H
#define SETMAP_H

#include <nodecpp/js_compat.h>
#include "./maps/america.h"
#include "./maps/rainbow.h"
#include "./maps/random.h"
#include "./maps/zebra.h"
#include "./custom/trap.h"




using namespace nodecpp::js;

class SetMap : public nodecpp::js::JSModule
{
public:
    JSVar exports() {
        return
            JSVar([](JSVar str, JSVar maps) {
            JSVar mappedStr = "";
            int i;
            if (maps == "america") {
                JSVar america = require<America>();
                JSOwnObj strChars = str.split("");
                str = strChars;
                for (i = 0; i < str.length(); i++) {
                    mappedStr += america(str[i], i);
                }
            }
            else if (maps == "rainbow") {
                    JSVar rainbow = require<Rainbow>();
                    JSOwnObj strChars = str.split("");
                    str = strChars;
                    for (i = 0; i < str.length(); i++) {
                        mappedStr += rainbow(str[i], i);
                    }
                }
            else if (maps == "random") {
                JSVar random = require<Random>();
                JSOwnObj strChars = str.split("");
                str = strChars;
                for (i = 0; i < str.length(); i++) {
                    mappedStr += random(str[i], i);
                }
            }
            else if (maps == "zebra") {
                JSVar zebra = require<Zebra>();
                JSOwnObj strChars = str.split("");
                str = strChars;
                for (i = 0; i < str.length(); i++) {
                    mappedStr += zebra(str[i], i);
                }
            }
            else if(maps == "trap"){
                JSVar trap = require<Trap>();
                mappedStr = trap(str);
            }
                return mappedStr;
            });
        }
    };
#endif // SETMAP_H