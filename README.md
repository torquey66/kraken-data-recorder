# Kraken Data Recorder

**Caveat emptor** *This is a work in progress - your mileage may vary.*

Kraken Data Recorder (*kdr*) is a tool for recording market data from
the Kraken crypto exchange (https://www.kraken.com/). It subscribes to
the *book* channel on the websocket v2 endpointand stores the content
in a series of parquet files which can later be used for analysis,
simulation, etc.

A distinguishing property of *kdr* is that it can perform this capture
with relatively little compute and memory overhead. For example, on my
modest Mac desktop it (3.1 GHz Intel Core i5) it can record the entire
*book* channel (aka all pairs) at full depth (1000) using under 30% of
a single core and less than 100MB memory. This includes performing the
[CRC32 checksum](https://docs.kraken.com/websockets-v2/#calculate-book-checksum)
for each book update.

See https://docs.kraken.com/websockets-v2/#introduction

## Running *kdr_record*

*kdr_record* supports these options.
```
bash-3.2$ ./kdr_record --help
Subscribe to Kraken and serialize book/trade data:
  --help                             display program options
  --ping_interval_secs arg (=30)     ping/pong delay
  --kraken_host arg (=ws.kraken.com) Kraken websocket host
  --kraken_port arg (=443)           Kraken websocket port
  --pair_filter arg (=[])            explicit list of pairs to subscribe to as 
                                     json string
  --parquet_dir arg (=/tmp)          directory in which to write parquet output
  --book_depth arg (=1000)           one of {10, 25, 100, 500, 1000}
  --capture_book arg (=1)            subscribe to and record level book
  --capture_trades arg (=1)          subscribe to and record trades
```

You can find prebuilt copies of *kdr_record* for Ubuntu in [releases](https://github.com/torquey66/kraken-data-recorder/releases).

## Building

### Prerequisites:

This project is currently built and tested with the following
tooling. These are by no means hard requirements (see for example the
CI pipeline for this project).

 - Conan version 2.5.0
 - cmake version 3.30.1
 - clang version 15.0.0
 - Python 3.12.4
 - jinja2-cli v0.8.2
 - Jinja2 v3.1.4

### Conan profile
```
[settings]
arch=x86_64
build_type=RelWithDebInfo
compiler=apple-clang
compiler.cppstd=gnu23
compiler.libcxx=libc++
compiler.version=15
os=Macos
```

### Command line build
```
mkdir build
conan install . --profile=release --build=missing --output-folder=build -sbuild_type=RelWithDebInfo
cd build
cmake  -DCMAKE_BUILD_TYPE=RelWithDebInfo  ..
make -j
ctest
```
