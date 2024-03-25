#include "AtlasMacros.hpp"
#include <algorithm>
#include <cassert>
#include <stdexcept>

#include "exchange/ExchangePrivate.hpp"
#include "exchange/Exchange.hpp"

namespace Atlas {

//==============================================================================
struct PyColDef {
  String _asset_id;
  String _column_name;
  size_t _matrix_index;

  PyColDef(String const &asset_id, String const &column_name,
           size_t matrix_index)
      : _asset_id(asset_id), _column_name(column_name),
        _matrix_index(matrix_index) {}
};

//==============================================================================
Exchange::Exchange(Vector<String> const &py_columns,
                   Vector<Int64> const &py_timestamps,
                   Vector<Vector<double>> const &data, size_t id) {
  // check that each column has exactly one space in the name
  HashMap<String, size_t> py_asset_names;
  Vector<PyColDef> py_column_defs;
  for (auto const &column : py_columns) {
    if (std::count(column.begin(), column.end(), ' ') != 1) {
      throw std::runtime_error("Column names must have exactly one space");
    }

    // get the asset name as the first word
    String asset_name = column.substr(0, column.find(' '));
    if (py_asset_names.find(asset_name) == py_asset_names.end()) {
      py_asset_names[asset_name] = py_asset_names.size();
    }

    // the column name is the second word
    String column_name = column.substr(column.find(' ') + 1);

    // add the column to py_column_defs
    PyColDef py_col_def(asset_name, column_name, py_column_defs.size());
    py_column_defs.push_back(py_col_def);
  }

  // get the column count, ensure that the division has no remainder
  if (py_column_defs.size() % py_asset_names.size() != 0) {
    throw std::runtime_error("Column count not a multiple of asset count");
  }
  size_t column_count = py_column_defs.size() / py_asset_names.size();

  // sort py_column_defs by name
  std::sort(
      py_column_defs.begin(), py_column_defs.end(),
      [](auto const &a, auto const &b) { return a._asset_id < b._asset_id; });

  // check that each asset has the same columns
  size_t i = 0;
  for (auto const &[asset_id, asset_index] : py_asset_names) {
    // find the first column in py_column_defs that matches the asset name
    auto it = std::find_if(py_column_defs.begin(), py_column_defs.end(),
                           [&asset_id](auto const &py_col) {
                             return py_col._asset_id == asset_id;
                           });
    assert(it != py_column_defs.end());

    // get distance from the first column to the first column of the next asset
    size_t distance = std::distance(it, py_column_defs.end());
    for (size_t j = distance; j < column_count; ++j) {
      auto const &column = py_column_defs[distance + j];
      if (column._column_name != py_column_defs[j]._column_name) {
        throw std::runtime_error(
            "Asset " + asset_id +
            " does not have the same columns as the other assets");
      }

      if (i == 0) {
        m_impl->headers[column._column_name] = j;
      }
    }
    ++i;
  }

  m_impl->col_count = column_count;

  Option<size_t> close_index = getCloseIndex();
  if (!close_index.has_value()) {
    throw std::runtime_error("No close column found");
  }
  m_impl->close_index = close_index.value();

  // build data matrix
  m_impl->data.resize(py_asset_names.size(),
                      py_timestamps.size() * m_impl->headers.size());
  // store the percentage change in price for each asset at each timestamp
  m_impl->returns.resize(m_impl->assets.size(), py_timestamps.size());

  // copy data from python to c++
  for (auto const &py_col : py_column_defs) {
    auto const &column = py_columns[py_col._matrix_index];
    size_t asset_index = py_asset_names[py_col._asset_id];
    size_t column_index = m_impl->headers[py_col._column_name];
    for (size_t i = 0; i < py_timestamps.size(); ++i) {
      size_t data_column_index = i * column_index;
      m_impl->data(asset_index, data_column_index) = column[i];
    }
  }
}

} // namespace Atlas