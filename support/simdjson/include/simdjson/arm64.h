#ifndef SIMDJSON_ARM64_H
#define SIMDJSON_ARM64_H

#include "implementation-base.h"

#if SIMDJSON_IMPLEMENTATION_ARM64

namespace simdjson {
/**
 * Implementation for NEON (ARMv8).
 */
namespace arm64 {
} // namespace arm64
} // namespace simdjson

#include "arm64/implementation.h"

#include "arm64/begin.h"

// Declarations
#include "generic/dom_parser_implementation.h"
#include "arm64/intrinsics.h"
#include "arm64/bitmanipulation.h"
#include "arm64/bitmask.h"
#include "arm64/simd.h"
#include "generic/jsoncharutils.h"
#include "generic/atomparsing.h"
#include "arm64/stringparsing.h"
#include "arm64/numberparsing.h"
#include "arm64/end.h"

#endif // SIMDJSON_IMPLEMENTATION_ARM64

#endif // SIMDJSON_ARM64_H
