// Platform dictionary of cached result for opportunistic information gathering
cached_results = {};
// Port array by platform dict
ports_by_platform = {};

urlmatch_platform = {
	"https:\/\/www.furaffinity.net\/view\/.*\/": "furaffinity.net"
};
function lookup_platform_from_url(url)
{
	for ([key, value] of Object.entries(urlmatch_platform)) {
		if (url.match(key))
			return value;
	};
	console.log("lookup_platform_from_url: Unknow platform \""+url+"\"");
	return null;
}

// Etablish connection with the native application on demand
var webext_adder_port_cache = null;
function webext_adder_port() {
	if (webext_adder_port_cache === null) {
		webext_adder_port_cache = browser.runtime.connectNative('arcollect_webext_adder');
	}
	return webext_adder_port_cache;
}

// Setup content-scripts calls
browser.runtime.onConnect.addListener(function(port) {
	port.arcollect_platform = lookup_platform_from_url(port.sender.url);
	console.log(`Content-script connection from ${port.sender.url} (${port.arcollect_platform})`);
	port.onMessage.addListener(function(msg) {
		webext_adder_port().postMessage(msg);
	});
});
