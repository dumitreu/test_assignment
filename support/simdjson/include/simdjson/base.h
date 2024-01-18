#ifndef SIMDJSON_BASE_H
#define SIMDJSON_BASE_H

#include "compiler_check.h"
#include "common_defs.h"
#include "portability.h"

SIMDJSON_PUSH_DISABLE_WARNINGS
SIMDJSON_DISABLE_UNDESIRED_WARNINGS

// Public API
#include "simdjson_version.h"
#include "error.h"
#include "minify.h"
#include "padded_string.h"
#include "padded_string_view.h"
#include "implementation.h"

// Inline functions
#include "error-inl.h"
#include "padded_string-inl.h"
#include "padded_string_view-inl.h"

SIMDJSON_POP_DISABLE_WARNINGS

#endif // SIMDJSON_BASE_H
