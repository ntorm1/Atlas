#pragma once

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b

#define NAN_DOUBLE std::numeric_limits<double>::quiet_NaN()

#define EXPECT_FALSE(expr, msg) \
    do { \
        if (expr) { \
            return std::unexpected<AtlasException>(msg); \
        } \
    } while (false)

#define Err(msg) std::unexpected<AtlasException>(msg)


#define SAFE_MAP_INSERT(map, key, value) \
    do { \
        try { \
            (map)[key] = value; \
        } catch (...) { \
            return std::unexpected<AtlasException>("MAP ALLOC FAILURE"); \
        } \
    } while (false)


#define EXPECT_TRUE(val, expr) \
	auto CONCAT(val, _opt) = expr; \
    if (!CONCAT(val, _opt)) { \
		return std::unexpected<Atlas::AtlasException>(CONCAT(val, _opt).error()); \
	} \

#define EXPECT_ALLOC(val, expr) \
    auto val##_opt = [&]() -> std::optional<decltype(expr)> { \
        try { \
            return (expr); \
        } catch (...) { \
            return std::nullopt; \
        } \
    }(); \
    \
    if (!val##_opt.has_value()) { \
        return std::unexpected<Atlas::AtlasException>("ALLOC FAILURE"); \
    } \
    \
    auto val = std::move(val##_opt.value());

#define EXPECT_ASSIGN(val, expr) \
	auto CONCAT(val, _opt) = expr; \
    if (!CONCAT(val, _opt)) { \
		return std::unexpected<Atlas::AtlasException>(CONCAT(val, _opt).error()); \
	} \
	auto val = std::move(CONCAT(val, _opt).value());


#define ATLAS_ASSIGN_OR_RETURN(val, function) \
	auto CONCAT(val, _opt) = function; \
    if (!CONCAT(val, _opt)) { \
		return std::unexpected<Atlas::AtlasException>(CONCAT(val, _opt).error()); \
	} \
	auto val = std::move(CONCAT(val, _opt).value());