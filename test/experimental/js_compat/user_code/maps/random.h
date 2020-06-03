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

#ifndef RANDOM_H
#define RANDOM_H
#include <nodecpp/js_compat.h>
#include "../styles.h"

using namespace nodecpp::js;

class Random : public nodecpp::js::JSModule
{
private:
    JSOwnObj avaible_ = makeJSArray({ "underline", "inverse", "grey", "yellow", "red", "green","blue",
                                  "white", "cyan", "magenta", "brightYellow", "brightRed","brightGreen", 
                                  "brightBlue", "brightWhite", "brightCyan", "brightMagenta" });
public:
    JSVar exports() {
        JSVar avaible = avaible_;
        return
                JSVar([avaible](JSVar letter, JSVar i) { 
                JSVar styles = require<Styles>();
                return letter.isStrictlyTheSame(" ") ? letter : styles(letter ,avaible[letter,JSMath::random() * (avaible.length() - 2)]);
                    });
            };
    };

// 
#endif // RANDOM_H

/*module['exports'] = function(colors) {
    var available = ['underline', 'inverse', 'grey', 'yellow', 'red', 'green',
        'blue', 'white', 'cyan', 'magenta', 'brightYellow', 'brightRed',
        'brightGreen', 'brightBlue', 'brightWhite', 'brightCyan', 'brightMagenta'];
    return function(letter, i, exploded) {
        return letter == = ' ' ? letter :
            colors[
                available[Math.round(Math.random() * (available.length - 2))]
            ](letter);
    };
};*/