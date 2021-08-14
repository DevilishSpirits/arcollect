# arcollect-webext-adder input/output format

The  `arcollect-webext-adder` is the program that handle WebExtension native host messaging. It communicate using JSON strings and add new artworks into the database.

# Query format
Here is an example showing all possible cases (unless I forgot something 🤔️) of a query to add something in the database.

When adding artwork, send the plain artwork with this kind of JSON :

```json
{
	"platform": "example.net",
	"transaction_id": "123456",
	"artworks": [{
		"title": "Sample art",
		"desc": "My sample warmup for the morning.",
		"source": "https://example.net/art/123456/",
		"rating": 0,
		"data": "<... base64 data ...>",
		"thumbnail": "<... base64 data ...>"
	}],
	"accounts": [{
		"id": 4213,
		"name": "@drawauthor",
		"title": "DrawAuthor make drawings",
		"url": "https://example.net/user/4213/",
		"icon": "<... base64 data ...>"
	}],
	"tags": [{
		"id": 2134,
		"title": "Dragon",
		"kind": "species"
	}],
	"art_acc_links": [{
		"artwork": "https://example.net/art/123456/",
		"account": 4213,
		"link": "account"
	}],
	"art_tag_links": [{
		"artwork": "https://example.net/art/123456/",
		"tag": 2134,
	}]
}
```
The `platform` is the platform identifier, the root URL of the platform like `twitter.com`. 

The `transaction_id` is a  string that is returned verbatim in the response to help the extension identify the tab destination. **Caution!** JSON escapes are not supported in this value, I still have to find how to espace or serialize JSON with RapidJSON.

The `artwork` array contain objects you wants to add with some properties :
* `title` is the artwork title.
* `desc` is the artwork description.
* `source` is the artwork URL. Caution ! This is a key in the database, reformat `window.location` in a way that the same artwork always have the same URL.
* `rating` is the artwork rating. See the schema explanation of `artworks` table in file [init.sql](https://github.com/DevilishSpirits/arcollect/blob/master/sql/init.sql).
* `mimetype` is the artwork mimetype. Try to guess the best MIME type but something like the default `image/*` is fine. When missing, Arcollect put `image/*` as default (in a future release it may record the HTTP [`Content-Type`](https://developer.mozilla.org/docs/Web/HTTP/Headers/Content-Typehttps://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Type) header).
* `postdate` is the UNIX timestamp of when the artworks has been posted (optional).
* `data` is the artwork file itself in base64 encoding or an `https://` link to the image (`http://` won't be supported).
* `thumbnail` is the image thumbnail to show for non image type artworks. Same format as `data`.

The `account` array contain users you might wants to add with some properties :
* `id` is the user internal id on the platform. It should be an immutable numeric id if available, or `name`. This is a key in the database. For example on Twitter `1294737021714550784`, you can use text.
* `name` is the username on the platform. Often something not very pretty with a limited charset. For example on Twitter `"@DevilishSpirits"`.
* `title` is the pretty username, if different from the `name` title. For example on Twitter `"D-Spirits"`.
* `url` is the account URL.
* `icon` is the account avatar in base64 encoding.

The `tags` array contain tags you might wants to add with some properties :
* `id` is the tag internal id on the platform. It should be immutable. This is a key in the database. It can be a string or an integer
* `title` is the pretty tag title, if different from `id`.
* `kind` is the tag kind. See the schema explanation of `tags` table in file [init.sql](https://github.com/DevilishSpirits/arcollect/blob/master/sql/init.sql).

The `art_acc_link` array contain links with artworks and profiles :
* `artwork` is the artwork `"source"` to link.
* `account` is the account `"id"`.
* `link` describre the type of link. Here `"account"` mean that this is the account which posted the artwork. Valid values are listed in the explanation of `art_acc_links` table in file [init.sql](https://github.com/DevilishSpirits/arcollect/blob/master/sql/init.sql).

The `art_tag_link` array contain links with artworks and tags :
* `artwork` is the artwork `"source"` to link.
* `tag` is the tag `"id"`.

This JSON is fully parsed before performing a transaction on the database. Items order in the JSON is not important.

## Result format

The program add artworks into the database and return an object when this is done :

```json
{
	"success": true,
	"transaction_id": "123456"
}
```

Or this failed :

```json
{
	"success": false,
	"reason": "A message describing what's wrong.",
	"transaction_id": "123456"
}
```

The `success` is self-explanatory.

The `reason` is a string describing what goes wrong that you should display to the user.

The `transaction_id` is the same JSON string that was sent in the corresponding request.
