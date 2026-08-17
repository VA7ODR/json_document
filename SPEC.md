# Temporary JSON specification notes

This file records compatibility questions observed while adding tests. It is
temporary: each item should become either a regression test with an explicit
decision or a source-code fix in a later change. The reference is RFC 8259
(JSON), with RFC 7493/I-JSON noted where relevant.

## Known or likely gaps

- **Top-level values:** The parser accepts scalar roots as well as objects and
  arrays.
- **Number grammar:** JSON forbids leading zeroes (`01`), requires digits after
  a decimal point (`1.` is invalid), and requires digits in an exponent
  (`1e+` is invalid). The parser intentionally accepts these permissively. JSON
  has no `NaN` or `Infinity` literals; the writer emits `null` for non-finite
  in-memory values.
- **Number range and precision:** RFC 8259 does not require arbitrary precision.
  The JSON writer serializes non-finite `double` values (`NaN` and infinities)
  as `null`, since JSON has no representation for them. Values outside the
  supported finite range still need a documented policy.
- **Strings and Unicode:** The parser intentionally accepts binary bytes,
  unescaped controls, unknown escapes, and malformed `\u` sequences inside a
  quoted value. Supported four-digit Unicode escapes and UTF-16 surrogate
  pairs are decoded as before.
- **UTF-8:** Input bytes are treated as data; the parser does not require valid
  UTF-8. The writer preserves valid UTF-8 and escapes unsupported bytes.
- **Object names and duplicates:** Names should be strings. RFC 8259 says names
  *should* be unique; duplicate-name behavior is implementation-dependent but
  must be documented (reject, first wins, last wins, or preserve all).
- **Whitespace:** ASCII whitespace recognized by `std::isspace` is accepted.
- **Trailing data:** Additional values and trailing commas are accepted; parsing
  `true false`, `[1,]`, and `{\"x\":1,}` succeeds.
- **Parser limits:** RFC 8259 leaves limits to implementations. Document any
  maximum nesting depth, input size, object member count, or array length, and
  ensure failures are reported without resource exhaustion.
- **Serialization escaping:** Writers escape quotation marks, reverse solidus,
  and all control characters; emitted text must be valid JSON and use valid
  Unicode escape sequences. Internal void sentinel values are intentionally
  omitted from output.

The permissive behavior above is intentional for compatibility with existing
binary and loosely formatted files.
