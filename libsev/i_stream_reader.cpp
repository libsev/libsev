/*

Copyright (C) 2017  by authors
Author: Jan Boon <support@no-break.space>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "i_stream_reader.h"

namespace sev {

IStreamReader::IStreamReader(EventFiber *ef, IStream *stream, size_t buffer = SEV_STREAM_READER_BUFFER_DEFAULT)
	: m_EventFiber(ef),
	m_Stream(stream), 
	m_UniqueBuffer(new char[buffer]), m_Buffer(m_UniqueBuffer.get()), 
	m_Index(0), m_Length(buffer),
	m_ReadError(false)
{
	
}

IStreamReader::IStreamReader(char *buffer, size_t index, size_t length)
	: m_EventFiber(NULL),
	m_Stream(NULL),
	m_Buffer(buffer),
	m_Index(index), m_Length(length),
	m_ReadError(false)
{
	
}

IStreamReader::IStreamReader(std::shared_ptr<char> buffer, size_t index, size_t length)
	: m_EventFiber(NULL),
	m_Stream(NULL),
	m_SharedBuffer(buffer), m_Buffer(buffer.get()),
	m_Index(index), m_Length(length),
	m_ReadError(false)
{
	
}

virtual ~IStreamReader()
{
	
}

std::string IStreamReader::readString()
{
	std::string res;
	const size_t size = readSize();
	res.resize(size); // WASTE: stl clears to 0, only need to clear first byte to 0 and last + 1 to zero
	res.resize(readBuffer(&res[0], 0, size)); // resize in case EOF
	return res; // implicit std::move
}

namespace /* anonymous */ {

void test()
{
	IStreamReader *sr;
	std::pair<std::string, int> v1 = sr->readPair<std::string, int>();
	std::pair<std::pair<std::string, int>, int> v2 = sr->readPair<std::pair<std::string, int>, int>();
}

} /* anonymous namespace */

} /* namespace sev */

/* end of file */
