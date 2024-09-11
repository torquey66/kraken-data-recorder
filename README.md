# Kraken Data Recorder

![sol_usd book animatino](doc/eth_usd.gif)

**Caveat emptor** *This project is under active development*

Kraken Data Recorder (*kdr*) is a tool for recording market data from
the Kraken crypto exchange (https://www.kraken.com/). It subscribes to
the *book* channel on the websocket v2 endpoint and stores the content
in a series of [parquet](https://parquet.apache.org/docs/file-format)
files which can later be used for analysis, simulation, etc. The
animation above, for example, was made from data captured by *kdr*,
1000 levels of the SOL/USD book.

*kdr* aims to capture the complete Kraken level book feed at full
depth for all symbols in a single process with a minimal
footprint. This includes performing the [CRC32
checksum](https://docs.kraken.com/websockets-v2/#calculate-book-checksum)
for each book update. 

See https://docs.kraken.com/websockets-v2/#introduction

## Quickstart

If you're familiar with Docker, the easiest way to try out *kdr* is by
running it in a container:

```/bin/bash
bash-3.2$  docker run -it --network=host  ghcr.io/torquey66/kraken-data-recorder:main
Unable to find image 'ghcr.io/torquey66/kraken-data-recorder:main' locally
main: Pulling from torquey66/kraken-data-recorder
eb993dcd6942: Pull complete 
69cd47597ee2: Pull complete 
Digest: sha256:cb49c0c7f1d2bbcdf5d43ee457a6bc93662b2527529531eacbdef074becf1b67
Status: Downloaded newer image for ghcr.io/torquey66/kraken-data-recorder:main
root@docker-desktop:/#
```

Once inside the container you can `kdr_record` some market data like this:
```/bin/bash
./kraken-data-recorder/build/kdr_record --book_depth=10 --pair_filter BTC/USD SOL/USD ETH/USD
```

This will record all `BTC/USD`, `SOL/USD`, and `ETH/USD` at
depth 10. Let it run for a bit and `ctrl-C` to terminate the
program. The output should look something like this:
```/bin/bash
root@docker-desktop:/# ./kraken-data-recorder/build/kdr_record --book_depth=10 --pair_filter BTC/USD SOL/USD ETH/USD 
[2024-09-10 20:23:54.585197] [0x00007f8b726e3800] [info]    kraken-data-recorder © 2023 by John C. Finley is licensed under Creative Commons Attribution-NoDerivatives 4.0 International. To view a copy of this license, visit https://creativecommons.org/licenses/by-nd/4.0/
[2024-09-10 20:23:54.585250] [0x00007f8b726e3800] [info]    starting up with config: {"book_depth":10,"capture_book":true,"capture_trades":true,"kraken_host":"ws.kraken.com","kraken_port":"443","pair_filter":["ETH/USD","SOL/USD","BTC/USD"],"parquet_dir":"/tmp","ping_interval_secs":30}
[2024-09-10 20:23:54.590222] [0x00007f8b726e3800] [info]    resolve suceeded
[2024-09-10 20:23:54.608940] [0x00007f8b726e3800] [info]    connect succeeded
[2024-09-10 20:23:54.629620] [0x00007f8b726e3800] [info]    ssl handshake succeeded
[2024-09-10 20:23:56.088102] [0x00007f8b726e3800] [info]    handshake succeeded
[2024-09-10 20:24:06.088550] [0x00007f8b726e3800] [info]    {"num_msgs":1678,"num_bytes":604858,"book_queue_depth":0,"book_max_queue_depth":20,"book_last_consumed":1,"book_last_process_micros":9,"num_heartbeats":10,"num_pings":0,"num_pongs":0}
[2024-09-10 20:24:16.089448] [0x00007f8b726e3800] [info]    {"num_msgs":3587,"num_bytes":976168,"book_queue_depth":0,"book_max_queue_depth":20,"book_last_consumed":1,"book_last_process_micros":9,"num_heartbeats":20,"num_pings":0,"num_pongs":0}
[2024-09-10 20:24:26.090441] [0x00007f8b726e3800] [info]    {"num_msgs":4997,"num_bytes":1251876,"book_queue_depth":0,"book_max_queue_depth":20,"book_last_consumed":6,"book_last_process_micros":20,"num_heartbeats":30,"num_pings":1,"num_pongs":0}
[2024-09-10 20:24:26.234504] [0x00007f8b726e3800] [info]    {"method":"pong","req_id":1,"time_in":"2024-09-10T20:24:26.280373Z","time_out":"2024-09-10T20:24:26.280397Z"}
[2024-09-10 20:24:36.090649] [0x00007f8b726e3800] [info]    {"num_msgs":6000,"num_bytes":1445750,"book_queue_depth":0,"book_max_queue_depth":20,"book_last_consumed":3,"book_last_process_micros":8,"num_heartbeats":40,"num_pings":1,"num_pongs":1}
[2024-09-10 20:24:46.091632] [0x00007f8b726e3800] [info]    {"num_msgs":7211,"num_bytes":1680544,"book_queue_depth":0,"book_max_queue_depth":20,"book_last_consumed":1,"book_last_process_micros":11,"num_heartbeats":50,"num_pings":1,"num_pongs":1}
^C[2024-09-10 20:24:50.369376] [0x00007f8b726e3800] [error]   received signal_number: 2 error: Success -- shutting down
[2024-09-10 20:24:50.369410] [0x00007f8b726e3800] [info]    session.stop_processing()
```

Results of the run will by default be left in `/tmp`:
```/bin/bash
root@docker-desktop:/# ls -l /tmp/*.pq
-rw-r--r-- 1 root root   5978 Sep 10 20:24 /tmp/1725999834586832.assets.pq
-rw-r--r-- 1 root root 252313 Sep 10 20:24 /tmp/1725999834586832.book.pq
-rw-r--r-- 1 root root  19200 Sep 10 20:24 /tmp/1725999834586832.pairs.pq
-rw-r--r-- 1 root root   6537 Sep 10 20:24 /tmp/1725999834586832.trades.pq
```
Each file is prefixed by a UTC timestamp in milliseconds. This allows
you to wrap *kdr_record* execution in a restart script to produce a
series of time-segmented files that can be combined after the fact.

You can take a peek at the capture using the `duckdb` utility,
included in the Docker image.
```/bin/bash
root@docker-desktop:/# ./duckdb 
v1.1.0 fa5c2fe15f
Enter ".help" for usage hints.
Connected to a transient in-memory database.
Use ".open FILENAME" to reopen on a persistent database.
D select * from read_parquet('/tmp/*.book.pq') limit 10;
┌──────────────────┬──────────┬──────────────────────┬─────────────────────────────────────────┬────────────┬─────────┬──────────────────┐
│     recv_tm      │   type   │         bids         │                  asks                   │  checksum  │ symbol  │    timestamp     │
│      int64       │ varchar  │ struct(price varch…  │  struct(price varchar, qty varchar)[]   │   uint64   │ varchar │      int64       │
├──────────────────┼──────────┼──────────────────────┼─────────────────────────────────────────┼────────────┼─────────┼──────────────────┤
│ 1725999836935616 │ snapshot │ [{'price': 57770.1…  │ [{'price': 57770.2, 'qty': 26.9509494…  │ 2820638371 │ BTC/USD │                0 │
│ 1725999836935641 │ snapshot │ [{'price': 2387.02…  │ [{'price': 2387.11, 'qty': 0.15325149…  │ 2178420705 │ ETH/USD │                0 │
│ 1725999836935657 │ snapshot │ [{'price': 136.20,…  │ [{'price': 136.21, 'qty': 0.15922645}…  │ 3213347641 │ SOL/USD │                0 │
│ 1725999836971483 │ update   │ [{'price': 57744.2…  │ []                                      │ 2803136326 │ BTC/USD │ 1725999837014760 │
│ 1725999836977233 │ update   │ [{'price': 57744.5…  │ []                                      │  982401181 │ BTC/USD │ 1725999837019996 │
│ 1725999836980007 │ update   │ []                   │ [{'price': 136.25, 'qty': 384.7172126…  │  887231010 │ SOL/USD │ 1725999837021538 │
│ 1725999836981043 │ update   │ [{'price': 136.17,…  │ []                                      │ 4106307791 │ SOL/USD │ 1725999837025648 │
│ 1725999836984317 │ update   │ []                   │ [{'price': 136.25, 'qty': 461.8836606…  │ 2059157472 │ SOL/USD │ 1725999837028122 │
│ 1725999836984387 │ update   │ [{'price': 136.19,…  │ []                                      │ 1936270871 │ SOL/USD │ 1725999837028596 │
│ 1725999836995023 │ update   │ []                   │ [{'price': 2387.23, 'qty': 104.724251…  │ 1424581938 │ ETH/USD │ 1725999837038709 │
├──────────────────┴──────────┴──────────────────────┴─────────────────────────────────────────┴────────────┴─────────┴──────────────────┤
│ 10 rows                                                                                                                      7 columns │
└────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```
All files contain snapshot as well as update events. These can be
replayed in an *event sourced* or *change data capture* fashion to
reproduce state at any given time.

 - **assets** contains the asset portion of the [instruments](https://docs.kraken.com/websockets-v2/#instrument)  reference data channel
 - **pairs** contains the asset portion of the [instruments](https://docs.kraken.com/websockets-v2/#instrument)  reference data channel
 - **book** contains snapshots and updates for all subscribed symbols on the [book](https://docs.kraken.com/websockets-v2/#book) channel
 - **trades** contains snapshots and updates for all subscribed symbols on the [trades](https://docs.kraken.com/websockets-v2/#trade) channel

## Running *kdr_record*

In addition to the aforementioned Docker image, you can find prebuilt
copies of *kdr_record* for Ubuntu in
[releases](https://github.com/torquey66/kraken-data-recorder/releases)
or build it yourself (see below).

### Options

*kdr_record* supports the following options:
```/bin/bash
bash-3.2$ kdr_record --help
Subscribe to Kraken and serialize book/trade data:
  --help                             display program options
  --ping_interval_secs arg (=30)     ping/pong delay
  --kraken_host arg (=ws.kraken.com) Kraken websocket host
  --kraken_port arg (=443)           Kraken websocket port
  --pair_filter arg                  pairs to record or empty to record all
  --parquet_dir arg (=/tmp)          directory in which to write parquet output
  --book_depth arg (=1000)           one of {10, 25, 100, 500, 1000}
  --capture_book arg (=1)            subscribe to and record level book
  --capture_trades arg (=1)          subscribe to and record trades
```

By default, it will capture all pairs at depth 1000 and create parquet
files in *parquet_dir*. Last I measured these could require on the
order of 1GB of storage per hour, so take care to set *parquet_dir* to
a location with ample space.

### Query examples

All queries below were made with the most excellent [duckdb](https://duckdb.org/) tool.

#### Assets

```/bin/bash
D select *  from read_parquet('/tmp/1725392418418599.assets.pq');
┌──────────────────┬────────────┬──────────────────┬──────────┬─────────────┬───────────┬───────────────────┬────────┐
│     recv_tm      │ borrowable │ collateral_value │    id    │ margin_rate │ precision │ precision_display │ status │
│      int64       │  boolean   │      double      │ varchar  │   double    │   int64   │       int64       │  int8  │
├──────────────────┼────────────┼──────────────────┼──────────┼─────────────┼───────────┼───────────────────┼────────┤
│ 1725392420846218 │ true       │              1.0 │ USD      │       0.025 │         4 │                 2 │      2 │
│ 1725392420846218 │ true       │              1.0 │ EUR      │        0.02 │         4 │                 2 │      2 │
│ 1725392420846218 │ true       │              1.0 │ GBP      │        0.02 │         4 │                 2 │      2 │
│ 1725392420846218 │ true       │              1.0 │ AUD      │        0.02 │         4 │                 2 │      2 │
│ 1725392420846218 │ true       │              1.0 │ CAD      │        0.02 │         4 │                 2 │      2 │
│         ·        │   ·        │               ·  │  ·       │      ·      │         · │                 · │      · │
│         ·        │   ·        │               ·  │  ·       │      ·      │         · │                 · │      · │
│         ·        │   ·        │               ·  │  ·       │      ·      │         · │                 · │      · │
│ 1725392420846218 │ false      │              0.0 │ USD.M    │             │         4 │                 4 │      2 │
│ 1725392420846218 │ false      │              0.0 │ EUR.M    │             │         4 │                 4 │      2 │
│ 1725392420846218 │ false      │              0.0 │ XBT.M    │             │        10 │                 8 │      2 │
│ 1725392420846218 │ false      │              0.0 │ USDT.M   │             │         8 │                 4 │      2 │
│ 1725392420846218 │ false      │              0.0 │ USDC.M   │             │         8 │                 4 │      2 │
├──────────────────┴────────────┴──────────────────┴──────────┴─────────────┴───────────┴───────────────────┴────────┤
│ 343 rows (40 shown)                                                                                      8 columns │
└────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

#### pairs
```/bin/bash
D select *  from read_parquet('/tmp/1725392418418599.pairs.pq');
┌──────────────────┬─────────┬──────────┬────────────────┬───────────┬────────────────┬────────────┬─────────────────────┬──────────────────────┬─────────────────┬─────────────────┬───────────────┬─────────┬───────────────┬─────────┬────────┬────────────┐
│     recv_tm      │  base   │ cost_min │ cost_precision │ has_index │ margin_initial │ marginable │ position_limit_long │ position_limit_short │ price_increment │ price_precision │ qty_increment │ qty_min │ qty_precision │  quote  │ status │   symbol   │
│      int64       │ varchar │  double  │     int64      │  boolean  │     double     │  boolean   │        int64        │        int64         │     double      │      int64      │    double     │ double  │     int64     │ varchar │  int8  │  varchar   │
├──────────────────┼─────────┼──────────┼────────────────┼───────────┼────────────────┼────────────┼─────────────────────┼──────────────────────┼─────────────────┼─────────────────┼───────────────┼─────────┼───────────────┼─────────┼────────┼────────────┤
│ 1725392420846218 │ EUR     │      0.5 │              5 │ true      │                │ false      │                     │                      │           1e-05 │               5 │         1e-08 │     0.5 │             8 │ USD     │      4 │ EUR/USD    │
│ 1725392420846218 │ GBP     │      0.5 │              5 │ true      │                │ false      │                     │                      │           1e-05 │               5 │         1e-08 │     5.0 │             8 │ USD     │      4 │ GBP/USD    │
│ 1725392420846218 │ USD     │     50.0 │              3 │ true      │                │ false      │                     │                      │           0.001 │               3 │         1e-08 │     5.0 │             8 │ JPY     │      4 │ USD/JPY    │
│ 1725392420846218 │ USD     │      1.0 │              5 │ true      │                │ false      │                     │                      │           1e-05 │               5 │         1e-08 │     5.0 │             8 │ CAD     │      4 │ USD/CAD    │
│ 1725392420846218 │ USD     │      0.5 │              5 │ true      │                │ false      │                     │                      │           1e-05 │               5 │         1e-08 │     5.0 │             8 │ CHF     │      4 │ USD/CHF    │
│         ·        │ ·       │       ·  │              · │  ·        │             ·  │  ·         │                  ·  │                   ·  │              ·  │               · │           ·   │      ·  │             · │  ·      │      · │   ·        │
│         ·        │ ·       │       ·  │              · │  ·        │             ·  │  ·         │                  ·  │                   ·  │              ·  │               · │           ·   │      ·  │             · │  ·      │      · │   ·        │
│         ·        │ ·       │       ·  │              · │  ·        │             ·  │  ·         │                  ·  │                   ·  │              ·  │               · │           ·   │      ·  │             · │  ·      │      · │   ·        │
│ 1725392420846218 │ TURBO   │     0.45 │             10 │ false     │                │ false      │                     │                      │           1e-06 │               6 │         1e-08 │  2000.0 │             8 │ EUR     │      4 │ TURBO/EUR  │
│ 1725392420846218 │ PRIME   │      0.5 │              7 │ false     │                │ false      │                     │                      │           0.001 │               3 │         1e-05 │     0.5 │             5 │ USD     │      4 │ PRIME/USD  │
│ 1725392420846218 │ PRIME   │     0.45 │              7 │ false     │                │ false      │                     │                      │           0.001 │               3 │         1e-05 │     0.5 │             5 │ EUR     │      4 │ PRIME/EUR  │
│ 1725392420846218 │ CXT     │      0.5 │              9 │ false     │                │ false      │                     │                      │           1e-05 │               5 │         1e-05 │    40.0 │             5 │ USD     │      4 │ CXT/USD    │
│ 1725392420846218 │ CXT     │     0.45 │              9 │ false     │                │ false      │                     │                      │           1e-05 │               5 │         1e-05 │    40.0 │             5 │ EUR     │      4 │ CXT/EUR    │
├──────────────────┴─────────┴──────────┴────────────────┴───────────┴────────────────┴────────────┴─────────────────────┴──────────────────────┴─────────────────┴─────────────────┴───────────────┴─────────┴───────────────┴─────────┴────────┴────────────┤
│ 753 rows (40 shown)                                                                                                                                                                                                                              17 columns │
└─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

#### book updates
```/bin/bash
D select *  from read_parquet('/tmp/*book.pq');
┌──────────────────┬──────────┬──────────────────────┬───────────────────────────────────────────────────────────┬────────────┬──────────┬──────────────────┐
│     recv_tm      │   type   │         bids         │                           asks                            │  checksum  │  symbol  │    timestamp     │
│      int64       │ varchar  │ struct(price varch…  │           struct(price varchar, qty varchar)[]            │   uint64   │ varchar  │      int64       │
├──────────────────┼──────────┼──────────────────────┼───────────────────────────────────────────────────────────┼────────────┼──────────┼──────────────────┤
│ 1725818865738784 │ snapshot │ [{'price': 0.84183…  │ [{'price': 0.84205, 'qty': 3617.04581098}, {'price': 0.…  │  606655977 │ USD/CHF  │                0 │
│ 1725818865740213 │ snapshot │ [{'price': 0.84481…  │ [{'price': 0.84489, 'qty': 4.11520286}, {'price': 0.844…  │ 2749081414 │ EUR/GBP  │                0 │
│ 1725818865740329 │ snapshot │ [{'price': 1.66588…  │ [{'price': 1.66979, 'qty': 264.97350000}, {'price': 1.6…  │  442136628 │ EUR/AUD  │                0 │
│ 1725818865748220 │ snapshot │ [{'price': 0.00556…  │ [{'price': 0.00557, 'qty': 0.01639996}, {'price': 0.005…  │ 2743151388 │ BCH/BTC  │                0 │
│ 1725818865749284 │ snapshot │ [{'price': 0.00000…  │ [{'price': 0.0000086, 'qty': 9684.19168542}, {'price': …  │ 2962860084 │ EOS/BTC  │                0 │
│         ·        │   ·      │          ·           │ ·                                                         │      ·     │    ·     │                · │
│         ·        │   ·      │          ·           │ ·                                                         │      ·     │    ·     │                · │
│         ·        │   ·      │          ·           │ ·                                                         │      ·     │    ·     │                · │
│ 1725819369569895 │ update   │ [{'price': 0.2917,…  │ []                                                        │ 2880461669 │ AEVO/EUR │ 1725819369652632 │
│ 1725819369569913 │ update   │ []                   │ [{'price': 1.00998, 'qty': 3976.20000000}]                │ 3983388217 │ USDT/USD │ 1725819369652848 │
│ 1725819369569956 │ update   │ []                   │ [{'price': 8.2282, 'qty': 0.00000}, {'price': 8.2283, '…  │  541797751 │ HNT/USD  │ 1725819369652768 │
│ 1725819369570606 │ update   │ []                   │ [{'price': 0.29999, 'qty': 0.00000}, {'price': 0.98570,…  │  651792773 │ SEI/USD  │ 1725819369654576 │
│ 1725819369570633 │ update   │ []                   │ [{'price': 0.00001336, 'qty': 2488884937.56192}]          │ 2383614753 │ SHIB/USD │ 1725819369654714 │
├──────────────────┴──────────┴──────────────────────┴───────────────────────────────────────────────────────────┴────────────┴──────────┴──────────────────┤
│ 2301326 rows (40 shown)                                                                                                                         7 columns │
└───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

#### trades
```/bin/bash
D select *  from read_parquet('/tmp/*trades.pq');
┌──────────────────┬──────────┬──────────┬────────────────┬─────────┬───────────┬──────────────────┬──────────┐
│     recv_tm      │ ord_type │  price   │      qty       │  side   │  symbol   │    timestamp     │ trade_id │
│      int64       │ varchar  │ varchar  │    varchar     │ varchar │  varchar  │      int64       │  uint64  │
├──────────────────┼──────────┼──────────┼────────────────┼─────────┼───────────┼──────────────────┼──────────┤
│ 1725818865150313 │ 2        │ 1.10757  │ 39.06953050    │ 1       │ EUR/USD   │ 1725818287524662 │ 25974101 │
│ 1725818865150313 │ 2        │ 1.10757  │ 6.04828588     │ 1       │ EUR/USD   │ 1725818287537778 │ 25974102 │
│ 1725818865150313 │ 2        │ 1.10757  │ 2.68714392     │ 1       │ EUR/USD   │ 1725818291981714 │ 25974103 │
│ 1725818865150313 │ 2        │ 1.10759  │ 21.34201283    │ 1       │ EUR/USD   │ 1725818297705538 │ 25974104 │
│ 1725818865150313 │ 2        │ 1.10759  │ 7.75604830     │ 1       │ EUR/USD   │ 1725818356153047 │ 25974105 │
│         ·        │ ·        │   ·      │     ·          │ ·       │    ·      │         ·        │     ·    │
│         ·        │ ·        │   ·      │     ·          │ ·       │    ·      │         ·        │     ·    │
│         ·        │ ·        │   ·      │     ·          │ ·       │    ·      │         ·        │     ·    │
│ 1725819366900646 │ 2        │ 0.84207  │ 89.79747949    │ 1       │ USD/CHF   │ 1725819366985251 │  4184340 │
│ 1725819366905024 │ 2        │ 0.153076 │ 586.58575000   │ 1       │ TRX/USD   │ 1725819366987404 │  2961118 │
│ 1725819367462839 │ 2        │ 0.00173  │ 7843.00000000  │ 1       │ ATLAS/USD │ 1725819367542882 │   441666 │
│ 1725819368060144 │ 2        │ 0.144    │ 465.44849389   │ 1       │ KILT/USD  │ 1725819368139194 │   365836 │
│ 1725819369460024 │ 2        │ 54376.2  │ 0.00535156     │ 1       │ BTC/USD   │ 1725819369416865 │ 73548806 │
├──────────────────┴──────────┴──────────┴────────────────┴─────────┴───────────┴──────────────────┴──────────┤
│ 39528 rows (40 shown)                                                                             8 columns │
└─────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

#### combining events and trades
Find updates that triggered trades.
```/bin/bash
D with src as (select * from read_parquet('/tmp/*book.pq') as book  join read_parquet('/tmp/*trades.pq') as trades on book.timestamp == trades.timestamp where book.symbol == trades.symbol) select timestamp,symbol,bids,asks,price as trade_price,qty as trade_qty,trade_id from src where symbol == 'SOL/USD';
┌──────────────────┬─────────┬──────────────────────┬─────────────────────────────────────────────┬─────────────┬─────────────┬──────────┐
│    timestamp     │ symbol  │         bids         │                    asks                     │ trade_price │  trade_qty  │ trade_id │
│      int64       │ varchar │ struct(price varch…  │    struct(price varchar, qty varchar)[]     │   varchar   │   varchar   │  uint64  │
├──────────────────┼─────────┼──────────────────────┼─────────────────────────────────────────────┼─────────────┼─────────────┼──────────┤
│ 1725819071033506 │ SOL/USD │ []                   │ [{'price': 128.95, 'qty': 0.99926988}]      │ 128.95      │ 0.00772000  │ 16445910 │
│ 1725819225060613 │ SOL/USD │ []                   │ [{'price': 128.94, 'qty': 2.54186000}]      │ 128.94      │ 0.13814000  │ 16445918 │
│ 1725818892035672 │ SOL/USD │ []                   │ [{'price': 128.95, 'qty': 77.54943776}, {…  │ 128.95      │ 0.01794960  │ 16445899 │
│ 1725818892035672 │ SOL/USD │ []                   │ [{'price': 128.95, 'qty': 77.54943776}, {…  │ 128.95      │ 0.04317040  │ 16445898 │
│ 1725819088587193 │ SOL/USD │ [{'price': 128.92,…  │ []                                          │ 128.92      │ 0.97053457  │ 16445911 │
│         ·        │ ·       │   ·                  │     ·                                       │ ·           │    ·        │     ·    │
│         ·        │ ·       │   ·                  │     ·                                       │ ·           │    ·        │     ·    │
│         ·        │ ·       │   ·                  │     ·                                       │ ·           │    ·        │     ·    │
│ 1725818870256081 │ SOL/USD │ [{'price': 128.98,…  │ []                                          │ 128.94      │ 22.19625521 │ 16445896 │
│ 1725818870256081 │ SOL/USD │ [{'price': 128.98,…  │ []                                          │ 128.95      │ 77.54749754 │ 16445895 │
│ 1725818870256081 │ SOL/USD │ [{'price': 128.98,…  │ []                                          │ 128.95      │ 5.91877699  │ 16445894 │
│ 1725818870256081 │ SOL/USD │ [{'price': 128.98,…  │ []                                          │ 128.95      │ 77.54360877 │ 16445893 │
│ 1725818870256081 │ SOL/USD │ [{'price': 128.98,…  │ []                                          │ 128.96      │ 0.06487253  │ 16445892 │
│ 1725818870256081 │ SOL/USD │ [{'price': 128.98,…  │ []                                          │ 128.98      │ 8.40000000  │ 16445891 │
├──────────────────┴─────────┴──────────────────────┴─────────────────────────────────────────────┴─────────────┴─────────────┴──────────┤
│ 31 rows                                                                                                                      7 columns │
└────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

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

### clang-tidy

Static checking via `clang-tidy` is currently a work in progress.
- There is a `.clang-tidy` file for the project which currently
  disables a few problematic checks.
- Over time this should evolve into a more complete set of appropriate
  checks.
- `clang-tidy` is not currently part of the CI pipeline.
- To run manually:
  - `cd build`
  - `cmake` with the `-DCMAKE_EXPORT_COMPILE_COMMANDS=1` flag
  - `clang-tidy -header-filter=.* -p=compile_commands.json $(find ../src -name '*.cpp')`


## License
kraken-data-recorder © 2023 by John C. Finley is licensed under
Creative Commons Attribution-NoDerivatives 4.0 International. To view
a copy of this license, visit
https://creativecommons.org/licenses/by-nd/4.0/
