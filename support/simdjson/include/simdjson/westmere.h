#ifndef SIMDJSON_WESTMERE_H
#define SIMDJSON_WESTMERE_H

#include "implementation-base.h"

#if SIMDJSON_IMPLEMENTATION_WESTMERE

#if SIMDJSON_CAN_ALWAYS_RUN_WESTMERE
#define SIMDJSON_TARGET_WESTMERE
#define SIMDJSON_UNTARGET_WESTMERE
#else
#define SIMDJSON_TARGET_WESTMERE SIMDJSON_TARGET_REGION("sse4.2,pclmul")
#define SIMDJSON_UNTARGET_WESTMERE SIMDJSON_UNTARGET_REGION
#endif

namespace simdjson {
/**
 * Implementation for Westmere (Intel SSE4.2).
 */
namespace westmere {
} // namespace westmere
} // namespace simdjson

//
// These two need to be included outside SIMDJSON_TARGET_WESTMERE
//
#include "westmere/implementation.h"
#include "westmere/intrinsics.h"

//
// The rest need to be inside the region
//
#include "westmere/begin.h"

// Declarations
#include "generic/dom_parser_implementation.h"
#include "westmere/bitmanipulation.h"
#include "westmere/bitmask.h"
#include "westmere/simd.h"
#include "generic/jsoncharutils.h"
#include "generic/atomparsing.h"
#include "westmere/stringparsing.h"
#include "westmere/numberparsing.h"
#include "westmere/end.h"

#endif // SIMDJSON_IMPLEMENTATION_WESTMERE
#endif // SIMDJSON_WESTMERE_COMMON_H
