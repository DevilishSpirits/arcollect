/* This SQL boostrap the database.
 *
 * It contain and explain the whole schema
 */
BEGIN;
	/* DB informations table
	 *
	 * This is a simple key/value pairs table holding metadata like the schema version.
	 */
	CREATE TABLE arcollect_infos (
		key   TEXT NOT NULL UNIQUE, /* The key */
		value TEXT                , /* The value */
		PRIMARY KEY (key)
	);
	INSERT INTO arcollect_infos (key,value) VALUES (
		'schema_version',1 /* Schema version - Used to upgrade the DB if needed */
	);
	
	/* Artwork database
	 *
	 * This is a repository of all collected artworks. It contain general metadata
	 * about the artwork and their unique id.
	 */
	CREATE TABLE artworks (
		art_artid    INTEGER NOT NULL UNIQUE, /* The artwork unique ID   */
		art_platform TEXT    NOT NULL       , /* The artwork platform    */
		art_title    TEXT                   , /* The artwork title       */
		art_desc     TEXT                   , /* The artwork description */
		art_source   TEXT    NOT NULL UNIQUE, /* The artwork source URL  */
		PRIMARY KEY (art_artid)
	);
COMMIT;
