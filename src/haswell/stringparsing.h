#ifndef SIMDJSON_HASWELL_STRINGPARSING_H
#define SIMDJSON_HASWELL_STRINGPARSING_H

#include "simdjson.h"
#include "jsoncharutils.h"
#include "haswell/simd.h"
#include "haswell/intrinsics.h"
#include "haswell/bitmanipulation.h"

TARGET_HASWELL
namespace simdjson::haswell {

using namespace simd;

// Holds backslashes and quotes locations.
struct backslash_and_quote {
public:
  static constexpr uint32_t BYTES_PROCESSED = 32;
  really_inline static backslash_and_quote copy_and_find(const uint8_t *src, uint8_t *dst);

  really_inline bool has_quote_first() { return ((bs_bits - 1) & quote_bits) != 0; }
  really_inline bool has_backslash() { return ((quote_bits - 1) & bs_bits) != 0; }
  really_inline int quote_index() { return trailing_zeroes(quote_bits); }
  really_inline int backslash_index() { return trailing_zeroes(bs_bits); }

  uint32_t bs_bits;
  uint32_t quote_bits;
}; // struct backslash_and_quote

really_inline backslash_and_quote backslash_and_quote::copy_and_find(const uint8_t *src, uint8_t *dst) {
  // this can read up to 15 bytes beyond the buffer size, but we require
  // SIMDJSON_PADDING of padding
  static_assert(SIMDJSON_PADDING >= (BYTES_PROCESSED - 1));
  simd8<uint8_t> v(src);
  // store to dest unconditionally - we can overwrite the bits we don't like later
  v.store(dst);
  return {
      (uint32_t)(v == '\\').to_bitmask(),     // bs_bits
      (uint32_t)(v == '"').to_bitmask(), // quote_bits
  };
}

#include "generic/stringparsing.h"

} // namespace simdjson::haswell
UNTARGET_REGION

#endif // SIMDJSON_HASWELL_STRINGPARSING_H