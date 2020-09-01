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

#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "common.h"
#include "common_structs.h"
#include <optional>

namespace nodecpp::fs
{
	class FD
	{
		friend FD openSync( nodecpp::string path, std::optional<nodecpp::string> flags, std::optional<nodecpp::string> mode );
		friend size_t readSync( FD fd, Buffer& b, size_t offset, size_t length, std::optional<size_t> position );
		friend void closeSync( FD fd );
		friend Buffer readFileSync( nodecpp::string path );
		FILE* fd = nullptr;
	};

	FD openSync( nodecpp::string path, std::optional<nodecpp::string> flags, std::optional<nodecpp::string> mode ) { 
		FD fd; 
		fd.fd = fopen( path.c_str(), flags.has_value() ? flags.value().c_str() : "r" ); 
		return fd;
	}

	size_t readSync( FD fd, Buffer& b, size_t offset, size_t length, std::optional<size_t> position ) { 
		if ( position.has_value() )
		{
			size_t currPos = ftell( fd.fd );
			fseek( fd.fd, position.value(), SEEK_SET );
			if ( b.capacity() < offset + length )
				b.reserve( offset + length );
			size_t ret = fread( b.begin() + offset, 1, length, fd.fd );
			fseek( fd.fd, currPos, SEEK_SET ); // restore
			if ( b.size() < offset + length )
				b.set_size( offset + length );
			return ret;
		}
		else
		{
			if ( b.capacity() < offset + length )
				b.reserve( offset + length );
			size_t ret = fread( b.begin() + offset, 1, length, fd.fd );
			if ( b.size() < offset + length )
				b.set_size( offset + length );
			return ret;
		}
	}

	Buffer readFileSync( nodecpp::string path ) { 
		auto fd = openSync( path, std::optional<string>(), std::optional<string>() );
		fseek( fd.fd, 0, SEEK_END ); // restore
		size_t sz = ftell( fd.fd );
		fseek( fd.fd, 0, SEEK_SET );
		Buffer b( sz );
		size_t actualSz = fread( b.begin(), 1, sz, fd.fd );
		b.set_size( sz );
		closeSync( fd );
		return b;
	}

	void closeSync( FD fd ) { if ( fd.fd ) fclose( fd.fd ); }

} // namespace nodecpp


#endif // FILE_SYSTEM_H
