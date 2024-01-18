#ifndef SIMDJSON_ICELAKE_H
#define SIMDJSON_ICELAKE_H

#include "implementation-base.h"

#if SIMDJSON_IMPLEMENTATION_ICELAKE

#if SIMDJSON_CAN_ALWAYS_RUN_ICELAKE
#define SIMDJSON_TARGET_ICELAKE
#define SIMDJSON_UNTARGET_ICELAKE
#else
#define SIMDJSON_TARGET_ICELAKE SIMDJSON_TARGET_REGION("avx512f,avx512dq,avx512cd,avx512bw,avx512vbmi,avx512vbmi2,avx512vl,avx2,bmi,pclmul,lzcnt")
#define SIMDJSON_UNTARGET_ICELAKE SIMDJSON_UNTARGET_REGION
#endif

namespace simdjson {
/**
 * Implementation for Icelake (Intel AVX512).
 */
namespace icelake {
} // namespace icelake
} // namespace simdjson

//
// These two need to be included outside SIMDJSON_TARGET_ICELAKE
//
#include "icelake/implementation.h"
#include "icelake/intrinsics.h"

//
// The rest need to be inside the region
//
#include "icelake/begin.h"

// Declarations
#include "generic/dom_parser_implementation.h"
#include "icelake/bitmanipulation.h"
#include "icelake/bitmask.h"
#include "icelake/simd.h"
#include "generic/jsoncharutils.h"
#include "generic/atomparsing.h"
#include "icelake/stringparsing.h"
#include "icelake/numberparsing.h"
#include "icelake/end.h"

#endif // SIMDJSON_IMPLEMENTATION_ICELAKE
#endif // SIMDJSON_ICELAKE_H
