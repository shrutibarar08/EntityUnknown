#pragma once
#include <exception>
#include <string>


class IException: public std::exception
{
public:
	IException(const std::string& fileName, int line);
	[[nodiscard]] const char* what() const override;

	virtual void PutMessage(const std::string& message);


private:
	std::string m_File{ "UNKNOWN" };
	int m_LINE{ 0 };
	std::string m_WhatBuffer{};
};
