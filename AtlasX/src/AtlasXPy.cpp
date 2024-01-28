
#include "../include/AtlasXPy.h"
#include "../include/AtlasXImpl.h"

import HydraModule;

namespace AtlasX
{

//============================================================================
Result<bool, Atlas::AtlasException>
	buildStrategy(
		AtlasXAppImpl* app,
		py::module& m) noexcept
{
	try {
		py::object result = m.attr("build")(app->getHydra());
	}
	catch (py::error_already_set& e) {
		return std::unexpected<Atlas::AtlasException>(e.what());
	}

	return true;
}

}

