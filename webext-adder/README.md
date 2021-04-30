# arcollect-webext-adder input/output format

The  `arcollect-webext-adder` is the program that handle WebExtension native host messaging. It communicate using JSON strings and add new artworks into the database.

# I/O format
Here is an example showing all possible cases (unless I forgot something 🤔️).

When adding artwork, send the plain artwork with this kind of JSON :
```json
{
	"platform": "example.net",
	"artwork": [{
		"title": "Sample art",
		"desc": "My sample warmup for the morning.",
		"source": "https://example.net/art/123456/",
		"data": "<... base64 data ...>"
	}]
}
```
The `platform` is the platform identifier, the root URL of the platform like `twitter.com`. 
The `artwork` array contain objects you wants to add with some properties :
* `title` is the artwork title.
* `desc` is the artwork description.
* `source` is the artwork URL. Caution ! This is a key in the database, reformat `window.location` in a way that the same artwork always have the same URL.
* `data` is the artwork file itself in base64 encoding.

The program add artworks into the database and return an empty object on this is done :
```json
{}
```
