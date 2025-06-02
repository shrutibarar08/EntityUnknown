#include "SweetLoader.h"

#include <algorithm>
#include <windows.h>
#include <string>
#include <sstream>
#include <iostream>

#include "Utils/Logger/Logger.h"


void SweetLoader::Load(const std::string& filePath)
{
	if (!m_FileSystem.OpenForRead(filePath))
		return;

	uint64_t fileSize = m_FileSystem.GetFileSize();
	if (fileSize == 0)
	{
		m_FileSystem.Close();
		return;
	}

	std::string content(fileSize, '\0');
	m_FileSystem.ReadBytes(&content[0], fileSize);
	m_FileSystem.Close();

	std::istringstream iss(content);
	FromStream(iss);
}

void SweetLoader::Save(const std::string& filepath)
{
	if (!m_FileSystem.OpenForWrite(filepath))
		return;

	std::ostringstream oss;
	Serialize(oss, 0); // convert into JSON-like string

	if (!m_FileSystem.WritePlainText(oss.str()))
	{
		LOG_ERROR("Failed To Save!");
	}
	m_FileSystem.Close();
}

SweetLoader& SweetLoader::operator=(const std::string& value)
{
	mValue = value;
	return *this;
}

const SweetLoader& SweetLoader::operator[](const std::string& key) const
{
	auto it = mChildren.find(key);
	if (it != mChildren.end())
		return it->second;

	static const SweetLoader invalidNode;  // Not connected to anything
	return invalidNode;
}

SweetLoader& SweetLoader::GetOrCreate(const std::string& key)
{
	return mChildren[key];
}

bool SweetLoader::Contains(const std::string& key) const
{
	return mChildren.contains(key);
}

std::string SweetLoader::ToFormattedString(int indent) const
{
	std::ostringstream oss;
	Serialize(oss, indent);
	return oss.str();
}

void SweetLoader::FromStream(std::istream& input)
{
	mChildren.clear();
	mValue.clear();

	ParseBlock(input);
}

void SweetLoader::Flatten(std::unordered_map<std::string, std::string>& out, const std::string& prefix) const
{
	if (!mChildren.empty())
	{
		for (const auto& [key, child] : mChildren)
		{
			std::string newPrefix = prefix.empty() ? key : prefix + "." + key;
			child.Flatten(out, newPrefix);
		}
	}
	else if (!mValue.empty())
	{
		out[prefix] = mValue;
	}
}

float SweetLoader::AsFloat() const
{
	if (!IsValid()) return 0.0f;

	try {
		return std::stof(mValue);
	}
	catch (...) {
		return 0.0f;
	}
}

int SweetLoader::AsInt() const
{
	if (!IsValid()) return 0;

	try {
		return std::stoi(mValue);
	}
	catch (...) {
		return 0;
	}
}

bool SweetLoader::AsBool() const
{
	if (!IsValid()) return false;

	std::string val = mValue;
	std::transform(val.begin(), val.end(), val.begin(), ::tolower);
	return (val == "true" || val == "1");
}


bool SweetLoader::IsValid() const

{
	return !mValue.empty() || !mChildren.empty();
}

void SweetLoader::Serialize(std::ostream& output, int indent) const
{
	const std::string indentStr(indent, '\t');

	if (!mChildren.empty())
	{
		output << "{\n";
		bool first = true;
		for (const auto& [key, child] : mChildren)
		{
			if (!first) output << ",\n";
			first = false;

			output << indentStr << '\t' << "\"" << key << "\": ";
			child.Serialize(output, indent + 1);
		}
		output << '\n' << indentStr << '}';
	}
	else
	{
		// Leaf node
		output << "\"" << mValue << "\"";
	}
}

void SweetLoader::ParseBlock(std::istream& input)
{
	auto skipWhitespace = [&](std::istream& in) {
		while (std::isspace(in.peek())) in.get();
		};

	auto readQuotedString = [&](std::istream& in) -> std::string {
		skipWhitespace(in);
		if (in.get() != '"') return {};
		std::string result;
		char c;
		while (in.get(c))
		{
			if (c == '"') break;
			result += c;
		}
		return result;
		};

	skipWhitespace(input);
	if (input.peek() != '{') return;
	input.get(); // consume '{'

	while (true)
	{
		skipWhitespace(input);
		if (input.peek() == '}')
		{
			input.get(); // consume '}'
			break;
		}

		std::string key = readQuotedString(input);

		skipWhitespace(input);
		if (input.get() != ':') return;

		skipWhitespace(input);
		if (input.peek() == '{')
		{
			// Nested block
			SweetLoader child;
			child.ParseBlock(input);
			mChildren[key] = std::move(child);
		}
		else if (input.peek() == '"')
		{
			std::string value = readQuotedString(input);
			mChildren[key].SetValue(value);
		}

		skipWhitespace(input);
		if (input.peek() == ',')
		{
			input.get(); // consume ','
			continue;
		}
		else if (input.peek() == '}')
		{
			continue; // handled by top
		}
		else
		{
			break; // Invalid format
		}
	}
}
