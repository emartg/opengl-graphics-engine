/*
 * File_System_Utils.h
 * This file defines the File_System_Utils class, which provides static utility methods
 * whose functionality revolves around the file system (e.g., locating the executable
 * or the resources directory), independently of the current working directory and platform.
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>

class File_System_Utils
{
public:
    // Public Static Methods
    // ---------------------
    // Returns the absolute path of the directory that contains the running executable
    // (or an empty path if it cannot be determined on the current platform)
    static std::filesystem::path get_executable_directory();

    // Returns the first existing directory from the given candidates (or an empty path if none exists)
    static std::filesystem::path find_first_existing_directory(const std::vector<std::filesystem::path>& candidates);
};