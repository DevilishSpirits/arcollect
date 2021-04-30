# Arcollect schemas

Arcollect store all data in a database at `$XDG_DATA_HOME/arcollect/db.sqlite3`.
The directory contain SQL which is used to create the database.

The schema itself is documented in `init.sql`, the file which bootstrap an empty
database.

The program `gen-sources.cpp` generate a pair of source/header that contain SQL
files wrapped.
