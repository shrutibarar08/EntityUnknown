#pragma once

#include <string>
#include <unordered_map>
#include <sstream>
#include "Utils/FileSystem/FileSystem.h"


class SweetLoader
{
public:
	// === Load / Save Entry Points ===
	void Load(const std::string& filePath);
	void Save(const std::string& filePath);

	// === Accessors ===
	SweetLoader& operator=(const std::string& value);
	const SweetLoader& operator[](const std::string& key) const;
	SweetLoader& GetOrCreate(const std::string& key);

	auto begin() { return mChildren.begin(); }
	auto end() { return mChildren.end(); }
	auto begin() const { return mChildren.begin(); }
	auto end()   const { return mChildren.end(); }

	const std::string& GetValue() const { return mValue; }
	void SetValue(const std::string& val) { mValue = val; }

	bool Contains(const std::string& key) const;

	// === Internal Helpers ===
	std::string ToFormattedString(int indent = 0) const; // Save to string in JSON format
	void FromStream(std::istream& input);               // Load from stream (basic parsing)

	// Optional utility: returns flattened map
	void Flatten(std::unordered_map<std::string, std::string>& out, const std::string& prefix = "") const;

	float AsFloat() const;
	int AsInt() const;
	bool AsBool() const;
	bool IsValid() const;

private:
	// === Private Recursive Parsers
	void Serialize(std::ostream& output, int indent) const;
	void ParseBlock(std::istream& input);

private:
	std::string mValue;
	std::unordered_map<std::string, SweetLoader> mChildren;
	FileSystem m_FileSystem{};
};
