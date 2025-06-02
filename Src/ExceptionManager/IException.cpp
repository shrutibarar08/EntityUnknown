#include "IException.h"

#include <sstream>

IException::IException(const std::string& fileName, int line)
	: m_File(fileName), m_LINE(line)
{
	std::string message{};

	message += "FILE_NAME [" + m_File + "]\n"
		+ "LINE_NUMBER [" + std::to_string(m_LINE) + "]\n";

	PutMessage(message);
}

const char* IException::what() const
{
    return m_WhatBuffer.c_str();
}

void IException::PutMessage(const std::string& message)
{
	m_WhatBuffer.append(message + "\n");
}
