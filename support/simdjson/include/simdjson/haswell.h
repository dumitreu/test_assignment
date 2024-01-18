#ifndef SIMDJSON_HASWELL_H
#define SIMDJSON_HASWELL_H

#include "implementation-base.h"

#if SIMDJSON_IMPLEMENTATION_HASWELL

#if SIMDJSON_CAN_ALWAYS_RUN_HASWELL
#define SIMDJSON_TARGET_HASWELL
#define SIMDJSON_UNTARGET_HASWELL
#else
#define SIMDJSON_TARGET_HASWELL SIMDJSON_TARGET_REGION("avx2,bmi,pclmul,lzcnt")
#define SIMDJSON_UNTARGET_HASWELL SIMDJSON_UNTARGET_REGION
#endif

namespace simdjson {
/**
 * Implementation for Haswell (Intel AVX2).
 */
namespace haswell {
} // namespace haswell
} // namespace simdjson

//
// These two need to be included outside SIMDJSON_TARGET_HASWELL
//
#include "haswell/implementation.h"
#include "haswell/intrinsics.h"

//
// The rest need to be inside the region
//
#include "haswell/begin.h"

// Declarations
#include "generic/dom_parser_implementation.h"
#include "haswell/bitmanipulation.h"
#include "haswell/bitmask.h"
#include "haswell/simd.h"
#include "generic/jsoncharutils.h"
#include "generic/atomparsing.h"
#include "haswell/stringparsing.h"
#include "haswell/numberparsing.h"
#include "haswell/end.h"

#endif // SIMDJSON_IMPLEMENTATION_HASWELL
#endif // SIMDJSON_HASWELL_COMMON_H
