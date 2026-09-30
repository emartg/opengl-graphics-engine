/*
 * File_System_Utils.cpp
 * This file implements the File_System_Utils class, which provides static utility methods
 * whose functionality revolves around the file system (e.g., locating the executable
 * or the resources directory), independently of the current working directory and platform.
 */

#include "File_System_Utils.h"

#include <system_error>

#if defined(_WIN32)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN // exclude rarely used Windows headers
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX // prevent the min and max macros from clashing with std::min and std::max
	#endif
	#include <windows.h>
#elif defined(__APPLE__)
	#include <mach-o/dyld.h>
	#include <cstdint>
#endif

// Public Static Methods
// ---------------------
std::filesystem::path File_System_Utils::get_executable_directory()
{
	std::error_code error; // used to avoid exceptions when the path cannot be resolved

#if defined(_WIN32)
	// query the executable path, growing the buffer until the whole path fits
	std::wstring buffer(MAX_PATH, L'\0');
	DWORD        length{ 0 };
	while ((length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()))) >= buffer.size())
	{
		buffer.resize(buffer.size() * 2);
	}
	if (length == 0)
		return {}; // the executable path could not be retrieved
	buffer.resize(length);
	std::filesystem::path executable_path{ buffer };
#elif defined(__APPLE__)
	// query the required buffer size first, then the executable path itself
	std::uint32_t size{ 0 };
	_NSGetExecutablePath(nullptr, &size);
	std::string buffer(size, '\0');
	if (_NSGetExecutablePath(buffer.data(), &size) != 0)
		return {};
	std::filesystem::path executable_path{ buffer.c_str() };
#elif defined(__linux__)
	// the kernel exposes the executable path as a symbolic link
	std::filesystem::path executable_path = std::filesystem::read_symlink("/proc/self/exe", error);
	if (error)
		return {};
#else
	return {}; // unsupported platform
#endif

	// resolve symbolic links and relative components, and return the parent directory
	std::filesystem::path canonical_path = std::filesystem::weakly_canonical(executable_path, error);
	return (error ? executable_path : canonical_path).parent_path();
}

std::filesystem::path File_System_Utils::find_first_existing_directory(const std::vector<std::filesystem::path>& candidates)
{
	std::error_code error; // used to avoid exceptions for inaccessible paths
	for (const auto& candidate : candidates)
	{ // return the first candidate that exists and is a directory
		if (!candidate.empty() && std::filesystem::is_directory(candidate, error))
		{ // normalize the path (e.g., "bin/../share"), keeping the original one if it cannot be resolved
			std::filesystem::path canonical_candidate = std::filesystem::weakly_canonical(candidate, error);
			return error ? candidate : canonical_candidate;
		}
	}
	return {};
}
