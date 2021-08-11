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
		('schema_version',2), /* Schema version - Used to upgrade the DB if needed */
		('bootstrap_date',strftime('%s','now')) /* Bootstrap date - When the database was bootstraped/the user started using Arcollect */
	;
	
	/* Artwork database
	 *
	 * This is a repository of all collected artworks. It contain general metadata
	 * about the artwork and their unique id.
	 *
	 * Note about art_width/art_height: Upon insertion, these values are set to
	 * NULL. The desktop-app will set these when loading the artwork.
	 *
	 * Note about art_rating: The rating works with a recomended minimal age
	 * inclusive (16 mean that 15 and less child are not recomended).
	 * The normalized rating are these :
	 * 	-  0 for unrated content
	 * 	- 16 for Mature content
	 * 	- 18 for Adult content
	 *
	 * Note about art_mimetype: This mime type is a somewhat relaxed thing. A mime
	 * of `image/*` is valid and tell Arcollect that it's an image, in fact just
	 * the image/ part tell Arcollect to load an artwork trough OpenImageIO but
	 * anyway, it's better to put the full mime-type (IF CORRECT!!!) in order to
	 * allow further nice features.
	 */
	CREATE TABLE artworks (
		art_artid    INTEGER NOT NULL UNIQUE, /* The artwork unique ID       */
		art_platform TEXT    NOT NULL       , /* The artwork platform        */
		art_width    INTEGER                , /* Artwork width in pixel      */
		art_height   INTEGER                , /* Artwork height in pixel     */
		art_title    TEXT                   , /* The artwork title           */
		art_desc     TEXT                   , /* The artwork description     */
		art_source   TEXT    NOT NULL UNIQUE, /* The artwork source URL      */
		art_rating   INTEGER                , /* The artwork rating age      */
		art_mimetype TEXT    NOT NULL       , /* The artwork mime-type       */
		art_postdate INTEGER                , /* When the artwork was posted */
		art_savedate INTEGER NOT NULL       DEFAULT (strftime('%s','now')), /* When the artwork was saved in Arcollect */
		PRIMARY KEY (art_artid)
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
		FOREIGN KEY (artu_artid) REFERENCES artworks(art_artid),
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
		acc_platform   TEXT    NOT NULL       , /* The account platform            */
		acc_name       TEXT                   , /* The account name                */
		acc_title      TEXT                   , /* The account title               */
		acc_url        TEXT    NOT NULL       , /* The account profile URL         */
		acc_createdate INTEGER                , /* When the account was created on the platform */
		acc_savedate   INTEGER NOT NULL       DEFAULT (strftime('%s','now')), /* When the account was saved in Arcollect */
		PRIMARY KEY (acc_arcoid)
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
		FOREIGN KEY (art_artid ) REFERENCES artworks(art_artid ),
		FOREIGN KEY (acc_arcoid) REFERENCES accounts(acc_arcoid),
		PRIMARY KEY (art_artid,acc_arcoid,artacc_link)
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
		FOREIGN KEY (art_artid ) REFERENCES artworks(art_artid ),
		FOREIGN KEY (tag_arcoid) REFERENCES tags    (tag_arcoid),
		PRIMARY KEY (art_artid,tag_arcoid)
	);
COMMIT;
