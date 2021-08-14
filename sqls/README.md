# Arcollect SQLs scripts

Arcollect store all metadata in a database at `$XDG_DATA_HOME/arcollect/db.sqlite3`.
This directory contain many SQL statements used for various operations and queries on the database.

The schema itself is documented in [`init.sql`](https://github.com/DevilishSpirits/arcollect/blob/master/sqls/init.sql) which also bootstrap the database upon first use.

The program [`gen-sources.cpp`](https://github.com/DevilishSpirits/arcollect/blob/master/sqls/gen-schema-sources.cpp) generate a pair of source/header with minified version of the SQL statements below.

* [`boot.sql`](https://github.com/DevilishSpirits/arcollect/blob/master/sqls/boot.sql) -- Pragmas runs at each database opening.
* [`delete_artwork.sql`](https://github.com/DevilishSpirits/arcollect/blob/master/sqls/delete_artwork.sql) -- Delete an artwork given his *art_artid*.
* [`init.sql`](https://github.com/DevilishSpirits/arcollect/blob/master/sqls/init.sql) -- Bootstrap an empty database for the first run.
* [`preload_artworks.sql`](https://github.com/DevilishSpirits/arcollect/blob/master/sqls/preload_artworks.sql) -- List artworks that should be preloaded even if not requested.
* [`upgrade_v2.sql`](https://github.com/DevilishSpirits/arcollect/blob/master/sqls/upgrade_v2.sql) -- Upgrade a [v1 schema](https://github.com/DevilishSpirits/arcollect/blob/v0.3/db-schema/init.sql) to the [v2 schema](](https://github.com/DevilishSpirits/arcollect/blob/v0.14/sqls/init.sql)), *probably something only me used to have*.
