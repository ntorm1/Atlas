// Save the current definition of slots
#ifdef slots
#define ORIGINAL_SLOTS_DEFINITION slots
#undef slots
#endif

#include <pybind11/embed.h>

// Restore the original definition of slots
#ifdef ORIGINAL_SLOTS_DEFINITION
#define slots ORIGINAL_SLOTS_DEFINITION
#endif

#include "../include/AtlasXTypes.h"

namespace py = pybind11;

import AtlasException;


namespace AtlasX
{
	//============================================================================
	Result<bool, Atlas::AtlasException> buildStrategy(
		AtlasXAppImpl* app,
		py::module& m
	) noexcept;
}