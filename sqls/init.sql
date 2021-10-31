/* Arcollect -- An artwork collection manager
 * Copyright (C) 2021 DevilishSpirits (aka D-Spirits or Luc B.)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/* This SQL boostrap the database.
 *
 * It contain and explain the whole schema.
 *
 * All dates in the schema are UNIX timestamps.
 */
BEGIN IMMEDIATE;
	/* DB informations table
	 *
	 * This is a simple key/value pairs table holding metadata like the schema version.
	 */
	CREATE TABLE arcollect_infos (
		key   TEXT NOT NULL UNIQUE, /* The key */
		value TEXT                , /* The value */
		PRIMARY KEY (key)
	);
	INSERT INTO arcollect_infos (key,value) VALUES
		('schema_version',3), /* Schema version - Used to upgrade the DB if needed */
		('bootstrap_date',strftime('%s','now')) /* Bootstrap date - When the database was bootstraped/the user started using Arcollect */
	;
	
	/* Downloaded files
	 *
	 * This is a table of all downloaded files. It allows Arcollect to perform
	 * HTTP caching.
	 * This is a kind of inode table to data-files with the rest of the database
	 * referencing them. This effectively allows Arcollect to have UNIX-inspired
	 * hard-links, this avoid some data duplication.
	 *
	 * Note about dwn_width/dwn_height: Upon insertion, these values are set to
	 * NULL. The desktop-app will set these when loading the artwork.
	 */
	CREATE TABLE downloads (
		dwn_id       INTEGER NOT NULL UNIQUE, /* Download unique ID           */
		dwn_source   TEXT             UNIQUE, /* Download URL                 */
		dwn_path     TEXT    NOT NULL UNIQUE, /* Download path                */
		dwn_mimetype TEXT    NOT NULL       , /* Download file type           */
		dwn_etag     TEXT                   , /* Download etag                */
		dwn_width    INTEGER                , /* Download width in pixels     */
		dwn_height   INTEGER                , /* Download height in pixels    */
		dwn_lastedit INTEGER NOT NULL       , /* Last edit time for If-Modified-Since */
		PRIMARY KEY (dwn_id)
	);
	
	/* Artwork database
	 *
	 * This is a repository of all collected artworks. It contain general metadata
	 * about the artwork and their unique id.
	 *
	 * Flags of art_flag0 (zero by default): 
	 * 1: Freeze artwork data. The source file will never be updated.
	 * The rest is reserved for future use.
	 *
	 * Note about art_rating: The rating works with a recomended minimal age
	 * inclusive (16 mean that 15 and less child are not recomended).
	 * The normalized rating are these :
	 * 	-  0 for unrated content
	 * 	- 16 for Mature content
	 * 	- 18 for Adult content
	 *
	 * Note about art_partof: this column have an AUTO INCREMENT behavior in sync
	 * with comics.com_arcoid and MAY be a reference to this comic, like a weak
	 * foreign key, there may be a linked entry in the comics table.
	 * The reason for that is that the pseudo-random hash of this column is a
	 * sorting key (with art_pageno) to allow comics grouping and that it is
	 * useless to create a comics entry in most cases.
	 *
	 * Note about art_pageno: This is a sorting key with the 64 bits word being
	 * divided in these parts : 0[part]4[pageno]36[subartwork]64
	 * * `part` Is the book part:
	 * * -9 for the `dummy` page numbering
	 * * -7 for the `front_cover`
	 * * -1 for the `prologue`
	 * *  0 for the `main` part
	 * * +1 for the `epilogue`
	 * * +6 for the `back_cover`
	 * * +7 for the `bonus`
	 * pageno is the page number, subartwork is used for illustrated books: 0 is
	 * used for the text and >1 for subsequent illustrations in the page.
	 */
	CREATE TABLE artworks (
		art_artid     INTEGER NOT NULL UNIQUE, /* The artwork unique ID       */
		art_dwnid     INTEGER NOT NULL       , /* The artwork download ID     */
		art_thumbnail INTEGER                , /* The dwn_id of the thumbnail */
		art_flag0     INTEGER NOT NULL       DEFAULT 0, /* First flag set     */
		art_platform  TEXT    NOT NULL       , /* The artwork platform        */
		art_title     TEXT                   , /* The artwork title           */
		art_desc      TEXT                   , /* The artwork description     */
		art_source    TEXT    NOT NULL UNIQUE, /* The artwork source URL      */
		art_rating    INTEGER                , /* The artwork rating age      */
		art_license   TEXT                   , /* Art SPDX license identifier */
		art_partof    INTEGER NOT NULL       , /* The comic it is part of     */
		art_pageno    INTEGER                , /* The position in the comic   */
		art_postdate  INTEGER                , /* When the artwork was posted */
		art_savedate  INTEGER NOT NULL       DEFAULT (strftime('%s','now')), /* When the artwork was saved in Arcollect */
		FOREIGN KEY (art_dwnid)      REFERENCES downloads(dwn_id),
		FOREIGN KEY (art_thumbnail)  REFERENCES downloads(dwn_id),
		PRIMARY KEY (art_artid)
	);
	
	/* Alternative sources
	 *
	 * This table list artwork alternative sources.
	 */
	CREATE TABLE artworks_altsources (
		art_artid     INTEGER NOT NULL, /* The artwork                */
		art_altsource TEXT    NOT NULL, /* The alternative source URL */
		FOREIGN KEY (art_artid) REFERENCES artworks(art_artid) ON DELETE CASCADE,
		PRIMARY KEY (art_artid,art_altsource)
	);
	
	/* Artwork not yet supported attributes
	 *
	 * This table complement artworks by saving generic key/value pairs of things
	 * that Arcollect do not support (yet).
	 *
	 * The format is per patform.
	 */
	CREATE TABLE artworks_unsupported (
		artu_artid    INTEGER NOT NULL UNIQUE, /* The artwork ID this relate to */
		artu_key      TEXT    NOT NULL       , /* The key                       */
		artu_value    TEXT                   , /* The value                     */
		FOREIGN KEY (artu_artid) REFERENCES artworks(art_artid) ON DELETE CASCADE,
		PRIMARY KEY (artu_artid,artu_key)
	);
	
	/* Artist account database
	 *
	 * This is a repository of all known artists accounts.
	 *
	 * Note about acc_platid: You should prefer immutable identifiers like numeric
	 * user ID when available.
	 */
	CREATE TABLE accounts (
		acc_arcoid     INTEGER NOT NULL UNIQUE, /* The account ID within Arcollect */
		acc_platid     INTEGER NOT NULL       , /* The account ID on the platform  */
		acc_icon       INTEGER NOT NULL       , /* The icon download ID            */
		acc_platform   TEXT    NOT NULL       , /* The account platform            */
		acc_name       TEXT                   , /* The account name                */
		acc_title      TEXT                   , /* The account title               */
		acc_desc       TEXT                   , /* The account description         */
		acc_url        TEXT    NOT NULL       , /* The account profile URL         */
		acc_moneyurl   TEXT                   , /* A link to tip the artist        */
		acc_createdate INTEGER                , /* When the account was created on the platform */
		acc_savedate   INTEGER NOT NULL       DEFAULT (strftime('%s','now')), /* When the account was saved in Arcollect */
		FOREIGN KEY (acc_icon) REFERENCES downloads(dwn_id),
		PRIMARY KEY (acc_arcoid)
	);
	
	/* Artist icon history
	 *
	 * This is a repository of artists known icons
	 */
	CREATE TABLE acc_icons (
		acc_arcoid     INTEGER NOT NULL       , /* The account ID within Arcollect */
		dwn_id         INTEGER NOT NULL       , /* The icon download ID            */
		ai_date        INTEGER NOT NULL       DEFAULT (strftime('%s','now')), /* When the icon was saved in Arcollect */
		FOREIGN KEY (acc_arcoid) REFERENCES accounts (acc_arcoid) ON DELETE CASCADE,
		FOREIGN KEY (dwn_id)     REFERENCES downloads(dwn_id)     ON DELETE CASCADE,
		PRIMARY KEY (acc_arcoid,dwn_id)
	);
	
	/* Artwork/account links
	 *
	 * These are links between artwork and accounts like who draw the art, using
	 * character of whom etc.
	 *
	 * Valid values for artacc_link are :
	 * - `account` -- The account that posted the art (author/commisionner).
	 * - `indesc` -- The account is mentionned in art_desc.
	 */
	CREATE TABLE art_acc_links (
		art_artid    INTEGER NOT NULL       , /* The artwork unique ID   */
		acc_arcoid   INTEGER NOT NULL       , /* The account ID within Arcollect */
		artacc_link  TEXT    NOT NULL       , /* The kind of link */
		FOREIGN KEY (art_artid ) REFERENCES artworks(art_artid ) ON DELETE CASCADE,
		FOREIGN KEY (acc_arcoid) REFERENCES accounts(acc_arcoid) ON DELETE CASCADE,
		PRIMARY KEY (art_artid,acc_arcoid,artacc_link)
	);
	
	/* Comic/account links
	 *
	 * These are links between comics and accounts like who draw made this comic.
	 *
	 * Valid values for artacc_link are :
	 * - `account` -- The account that posted the comic (author/comissionner).
	 */
	CREATE TABLE com_acc_links (
		com_arcoid   INTEGER NOT NULL       , /* The comic unique ID   */
		acc_arcoid   INTEGER NOT NULL       , /* The account ID within Arcollect */
		comacc_link  TEXT    NOT NULL       , /* The kind of link */
		FOREIGN KEY (com_arcoid) REFERENCES comics  (com_arcoid) ON DELETE CASCADE,
		FOREIGN KEY (acc_arcoid) REFERENCES accounts(acc_arcoid) ON DELETE CASCADE,
		PRIMARY KEY (com_arcoid,acc_arcoid,comacc_link)
	);
	
	/* Tags list
	 *
	 * This is a repository of all known tags and their meaning.
	 *
	 * Valid values for tag_kind are :
	 * - `species` -- The tag is related to a species
	 * - `character` -- The tag is related to a specific character
	 */
	CREATE TABLE tags (
		tag_arcoid     INTEGER NOT NULL UNIQUE, /* The tag ID within Arcollect */
		tag_platid     INTEGER NOT NULL       , /* The tag ID on the platform  */
		tag_platform   TEXT    NOT NULL       , /* The tag platform            */
		tag_title      TEXT                   , /* The tag title to display    */
		tag_kind       TEXT                   , /* The kind of tag             */
		tag_createdate INTEGER                , /* When the tag was created on the platform */
		tag_savedate   INTEGER NOT NULL       DEFAULT (strftime('%s','now')), /* When the tag was saved in Arcollect */
		PRIMARY KEY (tag_arcoid)
	);
	
	/* Artwork/tag links
	 *
	 * These are tags list on artworks.
	 */
	CREATE TABLE art_tag_links (
		art_artid    INTEGER NOT NULL       , /* The artwork unique ID   */
		tag_arcoid   INTEGER NOT NULL       , /* The tag ID within Arcollect */
		FOREIGN KEY (art_artid ) REFERENCES artworks(art_artid ) ON DELETE CASCADE,
		FOREIGN KEY (tag_arcoid) REFERENCES tags    (tag_arcoid) ON DELETE CASCADE,
		PRIMARY KEY (art_artid,tag_arcoid)
	);
	/* Comic/tag links
	 *
	 * These are tags list on comics.
	 */
	CREATE TABLE com_tag_links (
		com_arcoid   INTEGER NOT NULL       , /* The comic unique ID   */
		tag_arcoid   INTEGER NOT NULL       , /* The tag ID within Arcollect */
		FOREIGN KEY (com_arcoid) REFERENCES comics(com_arcoid) ON DELETE CASCADE,
		FOREIGN KEY (tag_arcoid) REFERENCES tags  (tag_arcoid) ON DELETE CASCADE,
		PRIMARY KEY (com_arcoid,tag_arcoid)
	);
	
	/* Comics metadata
	 *
	 * This table contain metadatas for "comics" (comics, manga, ...).
	 *
	 * There can still be comics on platforms which ignores what a comic is and
	 * there is no entry in this table but still an allocated com_arcoid.
	 */
	CREATE TABLE comics (
		com_arcoid     INTEGER NOT NULL UNIQUE, /* The comic ID within Arcollect */
		com_platid     INTEGER                , /* The comic ID on the platform  */
		com_platform   TEXT    NOT NULL       , /* The comic platform            */
		com_title      TEXT    NOT NULL       , /* The comic title               */
		com_url        TEXT                   , /* The comic URL                 */
		com_postdate   INTEGER                , /* When the comic was posted on the platform */
		com_savedate   INTEGER NOT NULL       DEFAULT (strftime('%s','now')), /* When the comic was saved in Arcollect */
		PRIMARY KEY (com_arcoid AUTOINCREMENT)
	);
	/* Forcibly init the sqlite_sequence in comics table */
	INSERT INTO sqlite_sequence(name,seq) VALUES ('comics',0);
	/* Comics missing pages
	 *
	 * This table contain metadatas for incomplete "comics" page we found, this
	 * allow Arcollect to link them automatically when they are finally saved.
	 */
	CREATE TABLE comics_missing_pages (
		art_artid      INTEGER NOT NULL, /* The reference artwork ID */
		cmp_source     TEXT    NOT NULL, /* The artwork source */
		cmp_delta      INTEGER NOT NULL, /* The page delta */
		FOREIGN KEY (art_artid) REFERENCES artworks(art_artid) ON DELETE CASCADE,
		PRIMARY KEY (art_artid,cmp_source)
	);
COMMIT;
