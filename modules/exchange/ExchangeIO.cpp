#include "AtlasFeature.hpp"
#include "AtlasMacros.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>
#ifdef ATLAS_HDF5
#include <H5Cpp.h>
#endif
#include "standard/AtlasTime.hpp"
#include "exchange/Exchange.hpp"
#include "exchange/ExchangePrivate.hpp"

namespace Atlas {

#ifdef ATLAS_HDF5
//============================================================================
static void loadH5(Asset &asset, H5::DataSet &dataset, H5::DataSpace &dataspace,
                   H5::DataSet &datasetIndex, H5::DataSpace &dataspaceIndex) {
  // Get the number of attributes associated with the dataset
  int numAttrs = dataset.getNumAttrs();
  // Iterate through the attributes to find the column names
  for (int i = 0; i < numAttrs; i++) {
    // Get the attribute at index i
    H5::Attribute attr = dataset.openAttribute(i);

    // Check if the attribute is a string type
    if (attr.getDataType().getClass() == H5T_STRING) {
      // Read the attribute as a string
      std::string attrValue;
      attr.read(attr.getDataType(), attrValue);

      // Store the attribute value as a column name
      asset.headers[attrValue] = static_cast<size_t>(i);
    }
  }

  // Get the number of rows and columns from the dataspace
  int numDims = dataspace.getSimpleExtentNdims();
  std::vector<hsize_t> dims(numDims);
  dataspace.getSimpleExtentDims(dims.data(), nullptr);
  auto rows = dims[0];
  auto cols = dims[1];
  asset.resize(rows, cols);
  dataset.read(asset.data.data(), H5::PredType::NATIVE_DOUBLE, dataspace);
  datasetIndex.read(asset.timestamps.data(), H5::PredType::NATIVE_INT64,
                    dataspaceIndex);
}
#endif

//============================================================================
Result<bool, AtlasException> Asset::loadCSV(String const &datetime_format) {
  assert(source);
  try {
    // Open the file
    std::ifstream file(*source);
    if (!file.is_open()) {
      return Err("Failed to open file: " + *source);
    }
    // get row count
    rows = 0;
    std::string line;
    while (std::getline(file, line)) {
      rows++;
    }
    rows--;
    file.clear();                 // Clear any error flags
    file.seekg(0, std::ios::beg); // Move the file pointer back to the start

    // Parse headers
    if (std::getline(file, line)) {
      std::stringstream ss(line);
      std::string columnName;
      int columnIndex = 0;

      // Skip the first column (date)
      std::getline(ss, columnName, ',');
      while (std::getline(ss, columnName, ',')) {
        headers[columnName] = columnIndex;
        columnIndex++;
      }
    } else {
      return Err("Could not parse headers");
    }
    cols = headers.size();
    resize(rows, cols);

    size_t row_counter = 0;
    while (std::getline(file, line)) {
      std::stringstream ss(line);

      // First column is datetime
      std::string timestamp, columnValue;
      std::getline(ss, timestamp, ',');

      // try to convert string to epoch time
      int64_t epoch_time = 0;
      if (datetime_format != "") {
        auto res = Time::strToEpoch(timestamp, datetime_format);
        if (res && res.value() > 0) {
          epoch_time = res.value();
        }
      } else {
        try {
          epoch_time = std::stoll(timestamp);
        } catch (...) {
        }
      }
      if (epoch_time == 0) {
        return Err("Invalid timestamp: " + timestamp +
                   ", epoch time is: " + std::to_string(epoch_time));
      }
      timestamps[row_counter] = epoch_time;

      int col_idx = 0;
      while (std::getline(ss, columnValue, ',')) {
        double value = std::stod(columnValue);
        size_t index = row_counter * cols + col_idx;
        data[index] = value;
        col_idx++;
      }
      row_counter++;
    }
    return true;
  } catch (const std::exception &e) {
    return Err("Error loading CSV: " + std::string(e.what()));
  } catch (...) {
    return Err("Error loading CSV: Unknown error");
  }
}

//============================================================================
Result<bool, AtlasException> Exchange::initDir() noexcept {
  // get all the files in the directory
  std::filesystem::path path(m_source);
  std::vector<std::filesystem::path> files;
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (entry.is_regular_file()) {
      files.push_back(entry.path());
    }
    if (entry.is_directory()) {
      String msg = "Exchange source directory contains subdirectories: " +
                   entry.path().string();
      return Err(msg);
    }
    if (entry.path().extension() != ".csv") {
      String msg = "Exchange source directory contains non-csv file: " +
                   entry.path().string();
      msg += ", found extension: " + path.extension().string() +
             ", expected: .csv";
      return Err(msg);
    }
    String asset_id = entry.path().stem().string();
    auto asset = Asset(asset_id, m_impl->assets.size());
    asset.setSource(entry.path().string());
    m_impl->assets.push_back(std::move(asset));
  }
  if (files.empty()) {
    return Err("Exchange source directory is empty");
  }

  // make sure have datetime format
  if (!m_impl->datetime_format) {
    return Err("Datetime format is required for loading CSV files");
  }

  // load the files
  std::vector<std::thread> threads;
  String msg = "";
  std::mutex m_mutex;
  for (auto &asset : m_impl->assets) {
    threads.push_back(std::thread([this, &asset, &m_mutex, &msg]() {
      auto res = asset.loadCSV(*(this->m_impl->datetime_format));
      if (!res) {
        std::lock_guard<std::mutex> lock(m_mutex);
        String error = res.error().what();
        String error_msg =
            std::format("Error loading asset: {} - {}\n", asset.id, error);
        msg += error_msg;
      }
    }));
  }

  // Join all threads
  for (auto &thread : threads) {
    thread.join();
  }

  if (!msg.empty()) {
    return Err(msg);
  }

  return true;
}

//============================================================================
Result<bool, AtlasException> Exchange::init() noexcept {
  // make sure the source string is file
  auto path = std::filesystem::path(m_source);
  EXPECT_FALSE(!std::filesystem::exists(path),
               "Exchange source file does not exist");

  if (std::filesystem::is_directory(path)) {
    return initDir();
  }

#ifdef ATLAS_HDF5
  // make sure the source file is an HDF5 file
  if (path.extension() != ".h5") {
    return Err("Exchange source is not an HDF5 file");
  }

  H5::H5File file(m_source, H5F_ACC_RDONLY);
  size_t numObjects = static_cast<size_t>(file.getNumObjs());
  for (size_t i = 0; i < numObjects; i++) {
    try {
      String asset_id = file.getObjnameByIdx(i);
      H5::DataSet dataset = file.openDataSet(asset_id + "/data");
      H5::DataSpace dataspace = dataset.getSpace();
      H5::DataSet datasetIndex = file.openDataSet(asset_id + "/datetime");
      H5::DataSpace dataspaceIndex = datasetIndex.getSpace();
      Asset asset(asset_id, m_impl->assets.size());
      m_impl->assets.push_back(std::move(asset));
      auto &asset_ref = m_impl->assets.back();
      loadH5(asset_ref, dataset, dataspace, datasetIndex, dataspaceIndex);
    } catch (H5::Exception &e) {
      return Err<AtlasException>("Error loading asset: " +
                                             std::string(e.getCDetailMsg()));
    } catch (const std::exception &e) {
      return Err<AtlasException>("Error loading asset: " +
                                             std::string(e.what()));
    } catch (...) {
      return Err<AtlasException>(
          "Error loading asset: Unknown error");
    }
  }
#else
  return Err<AtlasException>("HDF5 support is not enabled");
#endif

  return true;
}
} // namespace Atlas