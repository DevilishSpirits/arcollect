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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
/* This SQL upgrade a v1 database to the v2 format.
 */
/* Prepare upgrade */
PRAGMA foreign_keys = OFF;
/* Begin transaction */
BEGIN IMMEDIATE;
	/* Upgrade the artworks table */
	CREATE TABLE artworks_upgrade_v2 (
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
	INSERT OR ROLLBACK INTO artworks_upgrade_v2 (art_artid,art_platform,art_width,art_height,art_title,art_desc,art_source,art_rating,art_mimetype)
		SELECT art_artid,art_platform,art_width,art_height,art_title,art_desc,art_source,art_rating,'image/*' AS art_mimetype FROM artworks;
	DROP TABLE artworks;
	ALTER TABLE artworks_upgrade_v2 RENAME TO artworks;
	
	/* Upgrade the accounts table */
	CREATE TABLE accounts_upgrade_v2 (
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
	INSERT OR ROLLBACK INTO accounts_upgrade_v2 (acc_arcoid,acc_platid,acc_platform,acc_name,acc_title,acc_url)
		SELECT acc_arcoid,acc_platid,acc_platform,acc_name,acc_title,acc_url FROM accounts;
	DROP TABLE accounts;
	ALTER TABLE accounts_upgrade_v2 RENAME TO accounts;
	
	/* Upgrade the tags table */
	CREATE TABLE tags_upgrade_v2 (
		tag_arcoid     INTEGER NOT NULL UNIQUE, /* The tag ID within Arcollect */
		tag_platid     INTEGER NOT NULL       , /* The tag ID on the platform  */
		tag_platform   TEXT    NOT NULL       , /* The tag platform            */
		tag_title      TEXT                   , /* The tag title to display    */
		tag_kind       TEXT                   , /* The kind of tag             */
		tag_createdate INTEGER                , /* When the tag was created on the platform */
		tag_savedate   INTEGER NOT NULL       DEFAULT (strftime('%s','now')), /* When the tag was saved in Arcollect */
		PRIMARY KEY (tag_arcoid)
	);
	INSERT OR ROLLBACK INTO tags_upgrade_v2 (tag_arcoid,tag_platid,tag_platform,tag_title,tag_kind)
		SELECT tag_arcoid,tag_platid,tag_platform,tag_title,tag_kind FROM tags;
	DROP TABLE tags;
	ALTER TABLE tags_upgrade_v2 RENAME TO tags;
	
	
	/* Create the artworks_unsupported table */
	CREATE TABLE artworks_unsupported (
		artu_artid    INTEGER NOT NULL UNIQUE, /* The artwork ID this relate to */
		artu_key      TEXT    NOT NULL       , /* The key                       */
		artu_value    TEXT                   , /* The value                     */
		FOREIGN KEY (artu_artid) REFERENCES artworks(art_artid),
		PRIMARY KEY (artu_artid,artu_key)
	);
	
	/* Fix buggy tag_platid */
	UPDATE OR ROLLBACK tags SET tag_platid = (replace(tag_platid,' ','-'));
	UPDATE OR ROLLBACK tags SET tag_platid = (replace(tag_platid,'_','-'));
	
	/* Write the new DB version */
	INSERT OR REPLACE INTO arcollect_infos (key,value) VALUES ('schema_version',2);

/* Finish transaction and cleanups */
COMMIT;
PRAGMA foreign_keys = ON;
