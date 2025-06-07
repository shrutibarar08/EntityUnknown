#pragma once
#include <exception>
#include <string>

/*
	Base Class for Exceptions
*/
class IException: public std::exception
{
public:
	IException(const std::string& fileName, int line);
	[[nodiscard]] const char* what() const override;

	/*
		Put Error Message to be displayed by 'what'
	*/
	virtual void PutMessage(const std::string& message);

private:
	std::string m_File{ "UNKNOWN" };
	int m_LINE{ 0 };
	std::string m_WhatBuffer{};
};
