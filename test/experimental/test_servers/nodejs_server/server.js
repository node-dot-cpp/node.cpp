var http = require('http'), url = require('url');
http.createServer(function(request, response) {
    response.writeHead(200, {"Content-Type":"text/xml"});
    var urlObj = url.parse(request.url, true);
    var value = urlObj.query["value"];
    if (value == ''){
        response.end("no value specified");
    } else {
        response.end("" + value + "");
    }
}).listen(2000);