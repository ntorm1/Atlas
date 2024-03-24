#pragma once
#include <filesystem>
#include "AtlasXTypes.hpp"

namespace fs = std::filesystem;

BEGIN_NAMESPACE_ATLASX

static void deleteFilesRecursively(const fs::path &directory) {
  for (const auto &entry : fs::directory_iterator(directory)) {
    if (fs::is_directory(entry)) {
      // If it's a directory, recursively call the function
      deleteFilesRecursively(entry.path());
      // After deleting files in the directory, remove the directory itself
      fs::remove(entry);
    } else if (fs::is_regular_file(entry)) {
      // If it's a file, delete it
      fs::remove(entry);
    }
  }
}


static String const &componentTypeToString(ComponentType type) noexcept {
  static String const file_browser = "file_browser";
  switch (type) {
  case FILE_BROWSER:
    return file_browser;
  default:
    return file_browser;
  }
}

END_NAMESPACE_ATLASX