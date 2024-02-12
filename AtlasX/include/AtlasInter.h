#pragma once

#include "AtlasXTypes.h"

namespace AtlasX
{

//============================================================================
struct GridState
{
	String measure = "Measure";
	String dim1_name = "Dim1";
	String dim2_name = "Dim2";
	Vector<double> dim1;
	Vector<double> dim2;
	Vector<Vector<double>> data;

	GridState(
		String measure,
		String dim1_name,
		String dim2_name,
		Vector<double> dim1,
		Vector<double> dim2,
		Vector<Vector<double>> data
	) noexcept:
		measure(measure),
		dim1_name(dim1_name),
		dim2_name(dim2_name),
		data(std::move(data)),
		dim1(std::move(dim1)),
		dim2(std::move(dim2))
	{
	}

	bool hasData() const noexcept
	{
		return !data.empty();
	}

};




}