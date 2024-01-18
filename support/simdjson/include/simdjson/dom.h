#ifndef SIMDJSON_DOM_H
#define SIMDJSON_DOM_H

#include "base.h"

SIMDJSON_PUSH_DISABLE_WARNINGS
SIMDJSON_DISABLE_UNDESIRED_WARNINGS

#include "dom/array.h"
#include "dom/document_stream.h"
#include "dom/document.h"
#include "dom/element.h"
#include "dom/object.h"
#include "dom/parser.h"
#include "dom/serialization.h"

// Deprecated API
#include "dom/jsonparser.h"
#include "dom/parsedjson.h"
#include "dom/parsedjson_iterator.h"

// Inline functions
#include "dom/array-inl.h"
#include "dom/document_stream-inl.h"
#include "dom/document-inl.h"
#include "dom/element-inl.h"
#include "dom/object-inl.h"
#include "dom/parsedjson_iterator-inl.h"
#include "dom/parser-inl.h"
#include "internal/tape_ref-inl.h"
#include "dom/serialization-inl.h"

SIMDJSON_POP_DISABLE_WARNINGS

#endif // SIMDJSON_DOM_H
