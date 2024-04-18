# krakpot - Kraken PrOTotype

This is an exploratory prototype I'm building to better understand the
behavior of the Kraken crypto exchange, in particular its market data
feed. I hope to answer questions along the lines of:

 - How much resource is required to process the complete feed? Is this
   easily accomplished on a single machine or is some sort of
   distributed approach required?
   
 - Can one derive a level book from the Executions channel that
   matches the Book channel? And if so, are there latency
   discrepencies between the two?
   
Ultimately, I hope to evolve this into something that can be used for
capturing data for model training and simulation purposes as well as
live trading.

See https://docs.kraken.com/websockets-v2/#introduction

## Current status

'krakpot' can currently connect to Kraken, download security reference
data and subscribe to all listed instrument pairs. It leverages
`simdjson` to parse messages into internal data structures. As things
stand now it merely logs these to stdout, but I hope to soon add
capabilities for:

- keeping internal level books based on updates
- validating those via the CRC32 check described here:  https://docs.kraken.com/websockets-v2/#maintaining-the-book
- adding persistence to Parquet files via Apache Arrow

## Building and running

In theory, building this should be simple given Conan (v2) and a
reasonably up to date CMake. On MacOS Ventura, I am able to to this by running:

```

conan install . --profile=default --build=missing --output-folder=build -sbuild_type=Debug
cd ./build
cmake -DLIBCXX_ENABLE_INCOMPLETE_FEATURES=ON --preset conan-debug ..
make -j
```

To run:
```
./build/Kraken
```

## Notes to self:
- boost crc32 exmaple:
```
#include <boost/crc.hpp>

uint32_t GetCrc32(const string& my_string) {
    boost::crc_32_type result;
    result.process_bytes(my_string.data(), my_string.length());
    return result.checksum();
}
```
