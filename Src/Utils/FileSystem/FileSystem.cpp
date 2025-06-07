#include "FileSystem.h"


bool FileSystem::OpenForRead(const std::string& path)
{
	std::wstring w_path = std::wstring(path.begin(), path.end());
	mHandle = CreateFile(
		w_path.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	mReadMode = true;
	return mHandle != INVALID_HANDLE_VALUE;
}

bool FileSystem::OpenForWrite(const std::string& path)
{
	//~ Separate File and Directory
	auto file = SplitPathFile(path);

	//~ Create Directory
	CreateDirectories(file.DirectoryNames);

	//~ 
	std::wstring w_path = std::wstring(path.begin(), path.end());
	mHandle = CreateFile(
		w_path.c_str(),
		GENERIC_WRITE,
		0,
		nullptr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	mReadMode = false;
	return mHandle != INVALID_HANDLE_VALUE;
}

void FileSystem::Close()
{
	if (mHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(mHandle);
		mHandle = INVALID_HANDLE_VALUE;
		mReadMode = false;
	}
}

bool FileSystem::ReadBytes(void* dest, size_t size) const
{
	if (!mReadMode || mHandle == INVALID_HANDLE_VALUE) return false;

	DWORD bytesRead = 0;
	return ReadFile(mHandle, dest, static_cast<DWORD>(size), &bytesRead, nullptr) && bytesRead == size;
}

bool FileSystem::WriteBytes(const void* data, size_t size) const
{
	if (mReadMode || mHandle == INVALID_HANDLE_VALUE) return false;

	DWORD bytesWritten = 0;
	return WriteFile(mHandle, data, static_cast<DWORD>(size), &bytesWritten, nullptr) && bytesWritten == size;
}

bool FileSystem::ReadUInt32(uint32_t& value) const
{
	return ReadBytes(&value, sizeof(uint32_t));
}

bool FileSystem::WriteUInt32(uint32_t value) const
{
	return WriteBytes(&value, sizeof(uint32_t));
}

bool FileSystem::ReadString(std::string& outStr) const
{
	uint32_t len;
	if (!ReadUInt32(len)) return false;

	std::string buffer(len, '\0');
	if (!ReadBytes(buffer.data(), len)) return false;

	outStr = std::move(buffer);

	return true;
}

bool FileSystem::WriteString(const std::string& str) const
{
	uint32_t len = static_cast<uint32_t>(str.size());
	return WriteUInt32(len) && WriteBytes(str.data(), len);
}

bool FileSystem::WritePlainText(const std::string& str) const
{
	if (mReadMode || mHandle == INVALID_HANDLE_VALUE) return false;

	DWORD bytesWritten = 0;
	std::string line = str + "\n";
	return WriteFile(mHandle, line.c_str(), static_cast<DWORD>(line.size()), &bytesWritten, nullptr);
}

uint64_t FileSystem::GetFileSize() const
{
	if (mHandle == INVALID_HANDLE_VALUE) return 0;

	LARGE_INTEGER size{};
	if (!::GetFileSizeEx(mHandle, &size)) return 0;

	return static_cast<uint64_t>(size.QuadPart);
}

bool FileSystem::IsOpen() const
{
	return mHandle != INVALID_HANDLE_VALUE;
}

bool FileSystem::IsPathExists(const std::wstring& path)
{
	DWORD attr = GetFileAttributes(path.c_str());
	return (attr != INVALID_FILE_ATTRIBUTES);
}

DIRECTORY_AND_FILE_NAME FileSystem::SplitPathFile(const std::string& fullPath)
{
	// Supports both '/' and '\\'
	size_t lastSlash = fullPath.find_last_of("/\\");
	if (lastSlash == std::string::npos)
	{
		// No folder, only filename
		return { "", fullPath };
	}

	return {
		fullPath.substr(0, lastSlash),
		fullPath.substr(lastSlash + 1)
	};
}

bool FileSystem::IsPathExists(const std::string& path)
{
	std::wstring w_path = std::wstring(path.begin(), path.end());
	return IsPathExists(w_path);
}

bool FileSystem::IsDirectory(const std::string& path)
{
	std::wstring w_path(path.begin(), path.end());
	DWORD attr = GetFileAttributes(w_path.c_str());

	return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

bool FileSystem::IsFile(const std::string& path)
{
	std::wstring w_path(path.begin(), path.end());
	DWORD attr = GetFileAttributes(w_path.c_str());

	return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

bool FileSystem::CopyFiles(const std::string& source, const std::string& destination, bool overwrite)
{
	if (!IsPathExists(source))
	{
		return false;
	}

	std::wstring srcW(source.begin(), source.end());
	std::wstring dstW(destination.begin(), destination.end());

	return CopyFile(srcW.c_str(), dstW.c_str(), overwrite);
}

bool FileSystem::MoveFiles(const std::string& source, const std::string& destination)
{
	if (!IsPathExists(source))
	{
		return false;
	}

	std::wstring srcW(source.begin(), source.end());
	std::wstring dstW(destination.begin(), destination.end());

	return MoveFileW(srcW.c_str(), dstW.c_str());
}