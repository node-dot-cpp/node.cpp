var fletcher16 = require('fletcher');
var http = require('http'), url = require('url');
http.createServer(function(request, response) {
    if ( request.method == "GET" || request.method == "HEAD" ) {
        response.writeHead(200, {"Content-Type":"text/xml"});
        var urlObj = url.parse(request.url, true);
        var value = urlObj.query["value"];
        if (value == ''){
            response.end("no value specified");
        } else {
            var checks = fletcher16(Buffer.from("" + value + ""));
            response.end("" + value + " (" + checks + ")");
        }
    } else {
        response.writeHead(405, "Method Not Allowed");
	response.end();
    }
}).listen(2000);