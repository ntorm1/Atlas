#pragma once
#include "AtlasCore.hpp"

namespace Atlas
{

//============================================================================
template <typename T>
Vector<T> sortedUnion(const Vector<T>& vec1, const Vector<T>& vec2)
{
	Vector<T> result;
	result.reserve(vec1.size() + vec2.size());

	size_t i = 0, j = 0;

	while (i < vec1.size() && j < vec2.size()) {
		if (vec1[i] < vec2[j]) {
			result.push_back(vec1[i]);
			++i;
		}
		else if (vec1[i] > vec2[j]) {
			result.push_back(vec2[j]);
			++j;
		}
		else {
			// Elements are equal, add only once
			result.push_back(vec1[i]);
			++i;
			++j;
		}
	}

	// Add remaining elements from both vectors
	while (i < vec1.size()) {
		result.push_back(vec1[i]);
		++i;
	}

	while (j < vec2.size()) {
		result.push_back(vec2[j]);
		++j;
	}

	return result;
}


//============================================================================
template <typename T>
bool isContinuousSubset(const Vector<T>& a, const Vector<T>& b) {
	// Empty vector is always a subset of any vector
	if (b.empty()) {
		return true;
	}

	// Iterate over vector a to find the starting point of the subset
	for (size_t i = 0; i < a.size(); ++i) {
		if (a[i] == b[0]) {
			// Found a potential starting point
			size_t j = 0;

			// Check if the subsequent elements match
			while (j < b.size() && (i + j) < a.size() && a[i + j] == b[j]) {
				++j;
			}

			// If all elements of b are matched continuously in a, return true
			if (j == b.size()) {
				return true;
			}
		}
	}
	// If no match is found
	return false;
}

}