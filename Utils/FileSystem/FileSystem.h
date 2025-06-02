#pragma once
#include <windows.h>
#include <string>


typedef struct FILE_PATH_INFO
{
	std::string DirectoryNames;
	std::string FileName;
} DIRECTORY_AND_FILE_NAME;

class FileSystem
{
public:
	FileSystem() = default;
	~FileSystem() = default;

	FileSystem(const FileSystem&) = default;
	FileSystem(FileSystem&&) = default;
	FileSystem& operator=(const FileSystem&) = default;
	FileSystem& operator=(FileSystem&&) = default;

	bool OpenForRead(const std::string& path);
	bool OpenForWrite(const std::string& path);
	void Close();

	bool ReadBytes(void* dest, size_t size) const;
	bool WriteBytes(const void* data, size_t size) const;

	bool ReadUInt32(uint32_t& value) const;
	bool WriteUInt32(uint32_t value) const;

	bool ReadString(std::string& outStr) const;
	bool WriteString(const std::string& str) const;
	bool WritePlainText(const std::string& str) const;

	uint64_t GetFileSize() const;
	bool IsOpen() const;

	//~ Utility
	static bool IsPathExists(const std::wstring& path);
	static bool IsPathExists(const std::string& path);
	static bool IsDirectory(const std::string& path);
	static bool IsFile(const std::string& path);

	static bool CopyFiles(const std::string& source, const std::string& destination, bool overwrite = true);
	static bool MoveFiles(const std::string& source, const std::string& destination);

	static DIRECTORY_AND_FILE_NAME SplitPathFile(const std::string& fullPath);

	template<typename... Args>
	static bool DeleteFiles(Args&&... args);

	template<typename... Args>
	static bool CreateDirectories(Args&&... args);

private:
	HANDLE mHandle = INVALID_HANDLE_VALUE;
	bool mReadMode = false;
};

template<typename ...Args>
inline bool FileSystem::DeleteFiles(Args&& ...args)
{
	bool allSuccess = true;

	auto tryDelete = [&](const auto& path)
		{
			std::wstring w_path(path.begin(), path.end());
			if (!DeleteFile(w_path.c_str())) allSuccess = false;
		};

	(tryDelete(std::forward<Args>(args)), ...); // Folding lets goo...
	return allSuccess;
}

template<typename ...Args>
inline bool FileSystem::CreateDirectories(Args&& ...args)
{
	bool allSuccess = true;

	auto tryCreate = [&](const auto& pathStr)
		{
			std::wstring w_path(pathStr.begin(), pathStr.end());

			std::wstring current;
			for (size_t i = 0; i < w_path.length(); ++i)
			{
				wchar_t ch = w_path[i];
				current += ch;

				if (ch == L'\\' || ch == L'/')
				{
					if (!current.empty() && !IsPathExists(current))
					{
						if (!CreateDirectory(current.c_str(),
							nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
						{
							allSuccess = false;
							return;
						}
					}
				}
			}

			// Final directory (if not ends with slash)
			if (!IsPathExists(current))
			{
				if (!CreateDirectory(current.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
					allSuccess = false;
			}
		};

	(tryCreate(std::forward<Args>(args)), ...);
	return allSuccess;
}