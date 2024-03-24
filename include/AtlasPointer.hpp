#pragma once
#include <gsl/gsl>


namespace Atlas
{
	template <typename T>
	using NotNullPtr = gsl::not_null<T*>;
}