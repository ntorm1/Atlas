#pragma once
#include "AtlasXTypes.hpp"
#include <filesystem>

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
  static const Map<ComponentType, std::string> typeMap = {
      {ComponentType::FILE_BROWSER, "File Browser"},
      {ComponentType::EDITOR, "Editor"},
      {ComponentType::NETWORK, "Network"}};
  assert(typeMap.contains(type));
  return typeMap.at(type);
}

END_NAMESPACE_ATLASX