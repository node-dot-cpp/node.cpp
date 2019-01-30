#ifndef SOCKET_REGISTRATION_COMMON_H
#define SOCKET_REGISTRATION_COMMON_H

#include <stdio.h>
#include <assert.h>
#include "mallocator.h"
#include "../../../include/nodecpp/socket.h"

void registerSocket( nodecpp::SocketO* sock );

#endif // SOCKET_REGISTRATION_COMMON_H