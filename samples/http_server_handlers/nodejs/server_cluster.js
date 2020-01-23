const cluster = require('cluster');
const http = require('http');
var url = require('url');
console.log(process.argv.length);

const numCPUs = require('os').cpus().length;
console.log('Num CPUs', numCPUs);
//const numCPUs = process.argv[2];
//console.log(process.argv[2]);
if (cluster.isMaster) {
  console.log(`Master ${process.pid} is running`);

  // Fork workers.
  for (let i = 0; i < numCPUs; i++) {
    cluster.fork();
  }

  cluster.on('exit', (worker, code, signal) => {
    console.log(`worker ${worker.process.pid} died`);
  });
} else {
  // Workers can share any TCP connection
  // In this case it is an HTTP server
  var server = http.createServer(function(request, response) {
    if ( request.method == "GET" || request.method == "HEAD" ) {
        response.writeHead(200, {"Content-Type":"text/xml"});
        var urlObj = url.parse(request.url, true);
        var value = urlObj.query["value"];
        if (value == ''){
            response.end("no value specified");
        } else if (value == 'close'){
            server.close();
            response.end("closing server...");
        } else {
            response.end("" + value + "");
        }
    } else {
        response.writeHead(405, "Method Not Allowed");
	response.end();
    }
  }).listen(2000);

  console.log(`Worker ${process.pid} started`);
}
