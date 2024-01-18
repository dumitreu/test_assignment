#ifndef SIMDJSON_FALLBACK_H
#define SIMDJSON_FALLBACK_H

#include "implementation-base.h"

#if SIMDJSON_IMPLEMENTATION_FALLBACK

namespace simdjson {
/**
 * Fallback implementation (runs on any machine).
 */
namespace fallback {
} // namespace fallback
} // namespace simdjson

#include "fallback/implementation.h"

#include "fallback/begin.h"

// Declarations
#include "generic/dom_parser_implementation.h"
#include "fallback/bitmanipulation.h"
#include "generic/jsoncharutils.h"
#include "generic/atomparsing.h"
#include "fallback/stringparsing.h"
#include "fallback/numberparsing.h"
#include "fallback/end.h"

#endif // SIMDJSON_IMPLEMENTATION_FALLBACK
#endif // SIMDJSON_FALLBACK_H
