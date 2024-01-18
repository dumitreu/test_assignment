#ifndef SIMDJSON_PPC64_H
#define SIMDJSON_PPC64_H

#include "implementation-base.h"

#if SIMDJSON_IMPLEMENTATION_PPC64

namespace simdjson {
/**
 * Implementation for ALTIVEC (PPC64).
 */
namespace ppc64 {
} // namespace ppc64
} // namespace simdjson

#include "ppc64/implementation.h"

#include "ppc64/begin.h"

// Declarations
#include "generic/dom_parser_implementation.h"
#include "ppc64/intrinsics.h"
#include "ppc64/bitmanipulation.h"
#include "ppc64/bitmask.h"
#include "ppc64/simd.h"
#include "generic/jsoncharutils.h"
#include "generic/atomparsing.h"
#include "ppc64/stringparsing.h"
#include "ppc64/numberparsing.h"
#include "ppc64/end.h"

#endif // SIMDJSON_IMPLEMENTATION_PPC64

#endif // SIMDJSON_PPC64_H
