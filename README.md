# sqlite3-uuid

[![Build Status](https://travis-ci.org/benwebber/sqlite3-uuid.svg?branch=master)](https://travis-ci.org/benwebber/sqlite3-uuid)
[![codecov](https://codecov.io/gh/benwebber/sqlite3-uuid/branch/master/graph/badge.svg)](https://codecov.io/gh/benwebber/sqlite3-uuid)

This SQLite extension implements functions for creating [RFC 4122](https://www.ietf.org/rfc/rfc4122.txt) compliant UUIDs.

## Installation

1. Install the necessary development headers. Mac OS X ships with SQLite and the development headers, but you may need to install them on Linux.

    * Debian / Ubuntu:

        ```
        sudo apt-get install libsqlite3-dev libssl-dev uuid-dev
        ```

    * Red Hat / CentOS:

        ```
        sudo yum install openssl-devel sqlite-devel uuid-devel
        ```

2. Build the extension. The build will produce a shared library for your platform under `dist/`.

    ```
    make
    ```

2. Load the extension using your SQLite API of choice. For example, in Python,

    ```python
    import sqlite3

    con = sqlite3.connect(':memory:')

    con.enable_load_extension(True)
    con.load_extension('/path/to/uuid')
    con.enable_load_extension(False)

    for row in con.execute('SELECT uuid4();'):
        print(row)
    ```

## Usage

### `uuid1()`

Generate a version 1 UUID, based on the time and host machine's MAC address.

```sql
SELECT uuid1();
d5a80b20-0d8f-11e5-b8cb-080027b6ec40
```

### `uuid3()`

Generate a version 3 (MD5) namespace UUID.

```
SELECT uuid3(uuid_ns_dns(), 'example.org');
04738bdf-b25a-3829-a801-b21a1d25095b
```

### `uuid4()`

Generate a version 4 (random) UUID.

```sql
SELECT uuid4();
1e607604-f360-4fa5-863a-bc91adc70bb9
```

### `uuid5()`

Generate a version 5 (SHA1) namespace UUID.

```
SELECT uuid3(uuid_ns_dns(), 'example.org');
aad03681-8b63-5304-89e0-8ca8f49461b5
```

### `uuid_nil()`

Generate the nil UUID.

```
SELECT uuid_nil();
00000000-0000-0000-0000-000000000000
```

### Namespaces

#### DNS

```
SELECT uuid_ns_dns();
6ba7b810-9dad-11d1-80b4-00c04fd430c8
```

#### OID

```
SELECT uuid_ns_oid();
6ba7b812-9dad-11d1-80b4-00c04fd430c8
```

#### URL

```
SELECT uuid_ns_url();
6ba7b811-9dad-11d1-80b4-00c04fd430c8
```

#### X500

```
SELECT uuid_ns_x500();
6ba7b814-9dad-11d1-80b4-00c04fd430c8
```

### Converting between types

#### `uuid_to_blob()`

Convert a TEXT UUID to a 16-byte BLOB.

```
sqlite3 -cmd '.load uuid' uuid.db 'SELECT uuid_to_blob("3dfe5c62-e543-46ff-a2e0-0b1017506be0");' |
  tr -d '\n' |
  hexdump -C
00000000  3d fe 5c 62 e5 43 46 ff  a2 e0 0b 10 17 50 6b e0  |=.\b.CF......Pk.|
00000010
```

### `uuid_to_text()`

Convert a 16-byte BLOB to a TEXT UUID.

```
SELECT uuid_to_text(uuid_to_blob('3dfe5c62-e543-46ff-a2e0-0b1017506be0'));
3dfe5c62-e543-46ff-a2e0-0b1017506be0
```

## Notes

This extension only supports Linux and Mac OS X at the moment.
