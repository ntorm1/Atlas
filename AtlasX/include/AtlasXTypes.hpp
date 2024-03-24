#pragma once
#include <gsl/gsl>
#include <memory>
#include <string>
#include <vector>
#include <format>
#include <unordered_map>

#define BEGIN_NAMESPACE_ATLASX namespace AtlasX {
#define END_NAMESPACE_ATLASX }


#define QFORMAT(fmt, ...)                                                      \
  QString::fromStdString(std::format(fmt, __VA_ARGS__))

#define ATLASX_DEBUG(fmt, ...)                                                      \
  do {                                                                         \
    qDebug() << QString::fromStdString(                                        \
        std::format("[{}:{}]: " fmt, __FILE__, __LINE__, __VA_ARGS__));        \
  } while (0)

#define ATLASX_CRITICAL(fmt, ...)                                                 \
  do {                                                                         \
    qDebug() << QString::fromStdString(                                        \
        std::format("[{}:{}]: " fmt, __FILE__, __LINE__, __VA_ARGS__));        \
  } while (0)

BEGIN_NAMESPACE_ATLASX

enum ComponentType {
  FILE_BROWSER,
};

template <typename T>
using NotNullPtr = gsl::not_null<T*>;

template <typename T>
using Vector = std::vector<T>;

template <typename T>
using UniquePtr = std::unique_ptr<T>;

template <typename T>
using SharedPtr = std::shared_ptr<T>;

template <typename K, typename V>
using Map = std::unordered_map<K, V>;

using String = std::string;

class AtlasXComponent;
class App;
class AtlasXFileBrowser;

END_NAMESPACE_ATLASX