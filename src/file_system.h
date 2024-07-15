/**
 ******************************************************************************
 * @file           : file_system.h
 * @author         : CodingRookie
 * @brief          : None
 * @attention      : None
 * @date           : 24-7-15
 ******************************************************************************
 */

#ifndef FACEFUSIONCPP_SRC_FILE_SYSTEM_H_
#define FACEFUSIONCPP_SRC_FILE_SYSTEM_H_

#include <filesystem>
#include <string>
#include <opencv2/opencv.hpp>

namespace Ffc {

class FileSystem {
public:
    FileSystem() = default;
    ~FileSystem() = default;
    static bool fileExists(const std::string &path);
    static bool isDirectory(const std::string &path);
    static bool isFile(const std::string &path);
    static bool isImage(const std::string &path);
    static std::string getFileNameFromURL(const std::string &url);
    static uintmax_t getFileSize(const std::string &path);
};

} // namespace Ffc

#endif // FACEFUSIONCPP_SRC_FILE_SYSTEM_H_