# krakpot - Kraken PrOTotype

See https://docs.kraken.com/websockets-v2/#introduction

## To do:

- Persist to Parquet files via Arrow.
- Enable both Debug and Release builds.
- Figure out better Decimal resolution and representation (see
  https://support.kraken.com/hc/en-us/articles/4521313131540-Price-decimal-precision)
- Make root certificate configurable.
- Add raw logging option to capture bytes read from wire (in Parquet?).

## Ideas
- ML experiments from Parquet files.
- Try emscripten/webasm.

## References
- boost crc32:
```
#include <boost/crc.hpp>

uint32_t GetCrc32(const string& my_string) {
    boost::crc_32_type result;
    result.process_bytes(my_string.data(), my_string.length());
    return result.checksum();
}
```
