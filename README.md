# sqlite3-uuid

This SQLite extension implements functions for creating [RFC 4122](https://www.ietf.org/rfc/rfc4122.txt) compliant UUIDs.

## Installation

1. Install the necessary development headers. Mac OS X ships with SQLite and the development headers, but you may need to install them on Linux.

    * Debian / Ubuntu:

        ```
        sudo apt-get install libsqlite3-dev uuid-dev
        ```

    * Red Hat / CentOS:

        ```
        sudo yum install sqlite-devel uuid-devel
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

### `uuid4()`

Generate a version 4 (random) UUID.

```sql
SELECT uuid4();
1e607604-f360-4fa5-863a-bc91adc70bb9
```

## Notes

This extension only supports Linux and Mac OS X at the moment.
