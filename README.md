# hostsdb
A very simple tool that parses hosts files into a SQLite 3 database

(Works only on Linux!)

## Usage:
```
hostsdb <hosts-file> <hosts-file>...
```
The resulting database file is called `hosts.sqlite3`.

## Example:
```
hostsdb /etc/hosts ~/hosts.txt
```

## Installation
You need GCC and SQLite 3 in your standard path. (On Arch Linux you can install it with `# pacman -Syu gcc sqlite`)
Then clone this repository and run `make`. That's it!

## Other examples:
```
# Get all .org domains in the db (1 per line)
sqlite3 hosts.sqlite3 "SELECT DISTINCT host_name FROM rules WHERE host_name LIKE '%.org' ORDER BY host_name;"

# Count the number of .gov domains in the db
sqlite3 hosts.sqlite3 "SELECT DISTINCT COUNT(1) FROM rules WHERE host_name LIKE '%.gov';"
```
