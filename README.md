# Kraken Data Recorder

**Caveat emptor** *This is a work in progress - your mileage may vary.*

Kraken Data Recorder (*kdr*) is a tool for recording market data from
the Kraken crypto exchange (https://www.kraken.com/). It subscribes to
the *book* channel on the websocket v2 endpoint and stores the content
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

You can find prebuilt copies of *kdr_record* for Ubuntu in
[releases](https://github.com/torquey66/kraken-data-recorder/releases)
or build it yourself (see below).

### Options

*kdr_record* supports these options:
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

By default, it will capture all pairs at full depth and create parquet
files in *parquet_dir*. Last I measured these could require on the
order of 1GB of storage per hour, so take care to set *parquet_dir* to
a location with ample space.

### Example
```
kdr_record --ping_interval_secs 5 --parquet_dir /tmp/example
```

will create a series of parquet files in */tmp/exampe*:

```
bash-3.2$ ls -l /tmp/example/
total 49584
-rw-r--r--  1 torquey  wheel      5896 Aug  8 08:09 1723122455735103.assets.pq
-rw-r--r--  1 torquey  wheel  23606521 Aug  8 08:09 1723122455735103.book.pq
-rw-r--r--  1 torquey  wheel     18816 Aug  8 08:09 1723122455735103.pairs.pq
-rw-r--r--  1 torquey  wheel   1024693 Aug  8 08:09 1723122455735103.trades.pq
```

Each file is prefixed by a UTC timestamp in milliseconds. This allows
you to wrap *kdr_record* execution in a restart script to produce a
series of time-segmented files that can be combined after the fact.

All files contain snapshot as well as update events. These can be
replayed in an *event sourced* or *change data capture* fashion to
repreduce state at any given time.

 - **assets** contains the asset portion of the [instruments](https://docs.kraken.com/websockets-v2/#instrument}  reference data channel
 - **pairs** contains the asset portion of the [instruments](https://docs.kraken.com/websockets-v2/#instrument}  reference data channel
 - **book** contains snapshots and updates for all subscribed symbols on the [book](https://docs.kraken.com/websockets-v2/#book) channel
 - **trades** contains snapshots and updates for all subscribed symbols on the [trades](https://docs.kraken.com/websockets-v2/#trade) channel

**TBD** - ducdb query examples

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

## License
kraken-data-recorder © 2023 by John C. Finley is licensed under
Creative Commons Attribution-NoDerivatives 4.0 International. To view
a copy of this license, visit
https://creativecommons.org/licenses/by-nd/4.0/
