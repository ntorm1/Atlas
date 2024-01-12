module;
#include "AtlasMacros.hpp"
#include <filesystem>
#include <H5Cpp.h>
module ExchangeModule;

import ExchangePrivateModule;

namespace Atlas
{

//============================================================================
static void
	loadH5(
		Asset& asset,
		H5::DataSet& dataset,
		H5::DataSpace& dataspace,
		H5::DataSet& datasetIndex,
		H5::DataSpace& dataspaceIndex
	)
{
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
	datasetIndex.read(asset.timestamps.data(), H5::PredType::NATIVE_INT64, dataspaceIndex);
}


//============================================================================
Result<bool, AtlasException>
	Exchange::init() noexcept
{
	// make sure the source string is file
	auto path = std::filesystem::path(m_source);
	EXPECT_FALSE(!std::filesystem::exists(path), "Exchange source file does not exist");
	EXPECT_FALSE(std::filesystem::is_directory(path), "Exchange source is a directory");

	// make sure the source file is an HDF5 file
	if (path.extension() != ".h5") {
		return Err("Exchange source is not an HDF5 file");
	}

	H5::H5File file(m_source, H5F_ACC_RDONLY);
	size_t numObjects = static_cast<size_t>(file.getNumObjs());
	for (size_t i = 0; i < numObjects; i++)
	{
		try {
			String asset_id = file.getObjnameByIdx(i);
			H5::DataSet dataset = file.openDataSet(asset_id + "/data");
			H5::DataSpace dataspace = dataset.getSpace();
			H5::DataSet datasetIndex = file.openDataSet(asset_id + "/datetime");
			H5::DataSpace dataspaceIndex = datasetIndex.getSpace();
			Asset asset(asset_id, m_impl->assets.size());
			m_impl->assets.push_back(std::move(asset));
			auto& asset_ref = m_impl->assets.back();
			loadH5(asset_ref, dataset, dataspace, datasetIndex, dataspaceIndex);
		}
		catch (H5::Exception& e) {
			return std::unexpected<AtlasException>("Error loading asset: " + std::string(e.getCDetailMsg()));
		}
		catch (const std::exception& e) {
			return std::unexpected<AtlasException>("Error loading asset: " + std::string(e.what()));
		}
		catch (...) {
			return std::unexpected<AtlasException>("Error loading asset: Unknown error");
		}
	}

	return true;
}
}