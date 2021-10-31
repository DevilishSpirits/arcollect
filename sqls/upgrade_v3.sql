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
/* This SQL upgrade a v2 database to the v3 format.
 */
/* Prepare upgrade */
PRAGMA foreign_keys = OFF;
/* Begin transaction */
BEGIN IMMEDIATE;
	/* Create the downloads table */
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
	/* Put artworks in the downloads table */
	INSERT OR ROLLBACK INTO downloads (
		dwn_id,
		dwn_path,
		dwn_mimetype,
		dwn_width,
		dwn_height,
		dwn_lastedit
	) SELECT
		art_artid,                            /* For speed, use art_artid */
		'artworks/'||cast(art_artid AS TEXT), /* The path is hard-coded */
		art_mimetype,                         /* No change there */
		art_width,                            /* No change there */
		art_height,                           /* No change there */
		art_savedate                          /* Set a reasonable value */
	FROM artworks;
	/* Put artworks thumbnails in the downloads table */
	INSERT OR ROLLBACK INTO downloads (
		dwn_path,
		dwn_mimetype,
		dwn_lastedit
	) SELECT
		'artworks/'||cast(art_artid AS TEXT)||'.thumbnail', /* The path is hard-coded */
		'image/*',                                          /* Assume an image */
		art_savedate                                        /* Set a reasonable value */
	FROM artworks WHERE SUBSTR(art_mimetype,0,6) != 'image';
	/* Put accounts in the downloads table */
	INSERT OR ROLLBACK INTO downloads (
		dwn_path,
		dwn_mimetype,
		dwn_lastedit
	) SELECT
		'account-avatars/'||cast(acc_arcoid AS TEXT), /* The path is hard-coded */
		'image/*',                                    /* Assume an image */
		acc_savedate                                  /* Set a reasonable value */
	FROM accounts;
	
	/* Upgrade the artworks table */
	CREATE TABLE artworks_upgrade_v3 (
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
	INSERT OR ROLLBACK INTO artworks_upgrade_v3 (
		art_artid,
		art_dwnid,
		art_thumbnail,
		art_partof,
		art_platform,
		art_title,
		art_desc,
		art_source,
		art_rating,
		art_postdate,
		art_savedate
	) SELECT
		art_artid,
		art_artid,
		art_artid,
		art_artid,
		art_platform,
		art_title,
		art_desc,
		art_source,
		art_rating,
		art_postdate,
		art_savedate
	FROM artworks;
	/* Set thumbnails */
	UPDATE artworks_upgrade_v3 AS new_artworks
		SET art_thumbnail = (
			SELECT dwn_id FROM downloads WHERE dwn_path = 'artworks/'||cast(art_artid AS TEXT)||'.thumbnail'
		) WHERE substr((
			SELECT art_mimetype FROM artworks as old_artworks WHERE old_artworks.art_artid = new_artworks.art_artid)
			,0,6) != 'image';
	
	DROP TABLE artworks;
	ALTER TABLE artworks_upgrade_v3 RENAME TO artworks;
	
	CREATE TABLE artworks_altsources (
		art_artid     INTEGER NOT NULL, /* The artwork                */
		art_altsource TEXT    NOT NULL, /* The alternative source URL */
		FOREIGN KEY (art_artid) REFERENCES artworks(art_artid) ON DELETE CASCADE,
		PRIMARY KEY (art_artid,art_altsource)
	);
	/* Upgrade the accounts table */
	CREATE TABLE accounts_upgrade_v3 (
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
	INSERT OR ROLLBACK INTO accounts_upgrade_v3 (
		acc_arcoid,
		acc_platid,
		acc_icon,
		acc_platform,
		acc_name,
		acc_title,
		acc_url,
		acc_createdate,
		acc_savedate
	) SELECT
		acc_arcoid,
		acc_platid,
		(SELECT dwn_id FROM downloads WHERE dwn_path = 'account-avatars/'||cast(acc_arcoid AS TEXT)),
		acc_platform,
		acc_name,
		acc_title,
		acc_url,
		acc_createdate,
		acc_savedate
	FROM accounts;
	DROP TABLE accounts;
	ALTER TABLE accounts_upgrade_v3 RENAME TO accounts;
	
	CREATE TABLE acc_icons (
		acc_arcoid     INTEGER NOT NULL       , /* The account ID within Arcollect */
		dwn_id         INTEGER NOT NULL       , /* The icon download ID            */
		ai_date        INTEGER NOT NULL       DEFAULT (strftime('%s','now')), /* When the icon was saved in Arcollect */
		FOREIGN KEY (acc_arcoid) REFERENCES accounts (acc_arcoid) ON DELETE CASCADE,
		FOREIGN KEY (dwn_id)     REFERENCES downloads(dwn_id)     ON DELETE CASCADE,
		PRIMARY KEY (acc_arcoid,dwn_id)
	);
	INSERT OR ROLLBACK INTO acc_icons (
		acc_arcoid,
		dwn_id,
		ai_date
	) SELECT
		acc_arcoid,
		acc_icon,
		acc_savedate
	FROM accounts;
	
	CREATE TABLE com_acc_links (
		com_arcoid   INTEGER NOT NULL       , /* The comic unique ID   */
		acc_arcoid   INTEGER NOT NULL       , /* The account ID within Arcollect */
		comacc_link  TEXT    NOT NULL       , /* The kind of link */
		FOREIGN KEY (com_arcoid) REFERENCES comics  (com_arcoid) ON DELETE CASCADE,
		FOREIGN KEY (acc_arcoid) REFERENCES accounts(acc_arcoid) ON DELETE CASCADE,
		PRIMARY KEY (com_arcoid,acc_arcoid,comacc_link)
	);
	CREATE TABLE com_tag_links (
		com_arcoid   INTEGER NOT NULL       , /* The comic unique ID   */
		tag_arcoid   INTEGER NOT NULL       , /* The tag ID within Arcollect */
		FOREIGN KEY (com_arcoid) REFERENCES comics(com_arcoid) ON DELETE CASCADE,
		FOREIGN KEY (tag_arcoid) REFERENCES tags  (tag_arcoid) ON DELETE CASCADE,
		PRIMARY KEY (com_arcoid,tag_arcoid)
	);
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
	INSERT INTO sqlite_sequence(name,seq) VALUES ('comics',0);
	
	CREATE TABLE comics_missing_pages (
		art_artid      INTEGER NOT NULL, /* The reference artwork ID */
		cmp_source     TEXT    NOT NULL, /* The artwork source */
		cmp_delta      INTEGER NOT NULL, /* The page delta */
		FOREIGN KEY (art_artid) REFERENCES artworks(art_artid) ON DELETE CASCADE,
		PRIMARY KEY (art_artid,cmp_source)
	);
	
	/* Write the new DB version */
	INSERT OR REPLACE INTO arcollect_infos (key,value) VALUES ('schema_version',3);

/* Finish transaction and cleanups */
COMMIT;
PRAGMA foreign_keys = ON;
