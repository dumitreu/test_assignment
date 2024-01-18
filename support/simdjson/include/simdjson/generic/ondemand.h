namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
/**
 * A fast, simple, DOM-like interface that parses JSON as you use it.
 *
 * Designed for maximum speed and a lower memory profile.
 */
namespace ondemand {

/** Represents the depth of a JSON value (number of nested arrays/objects). */
using depth_t = int32_t;

} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

#include "ondemand/json_type.h"
#include "ondemand/token_position.h"
#include "ondemand/logger.h"
#include "ondemand/raw_json_string.h"
#include "ondemand/token_iterator.h"
#include "ondemand/json_iterator.h"
#include "ondemand/value_iterator.h"
#include "ondemand/array_iterator.h"
#include "ondemand/object_iterator.h"
#include "ondemand/array.h"
#include "ondemand/document.h"
#include "ondemand/value.h"
#include "ondemand/field.h"
#include "ondemand/object.h"
#include "ondemand/parser.h"
#include "ondemand/document_stream.h"
#include "ondemand/serialization.h"
