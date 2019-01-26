# hostsdb
A very simple tool that parses a single hosts file into a SQLite 3 database

(Works only on Linux!)

## Usage:
```
hostsdb <hosts-file>
```
The resulting database file is called `hosts.sqlite3` and contains a single table called `rules` with two columns called `ip_address` and `host_name`.

## Example:
```
hostsdb /etc/hosts
```

## Installation
You need GCC and SQLite 3 in your standard path. (On Arch Linux you can install it with `# pacman -Syu gcc sqlite`)
Then clone this repository and run `make`. That's it!

## Convert the database back to a hosts file:
```
sqlite3 hosts.sqlite3 "SELECT ip_address || CHAR(9) || host_name FROM rules ORDER BY host_name;" | expand -t 24 > hosts
```

## Other examples:
```
# Get all .org domains in the db (1 per line)
sqlite3 hosts.sqlite3 "SELECT DISTINCT host_name FROM rules WHERE host_name LIKE '%.org' ORDER BY host_name;"

# Count the number of .gov domains in the db
sqlite3 hosts.sqlite3 "SELECT DISTINCT COUNT(1) FROM rules WHERE host_name LIKE '%.gov';"
```

