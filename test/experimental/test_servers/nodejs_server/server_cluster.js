const cluster = require('cluster');
const http = require('http');
var url = require('url');
// const numCPUs = require('os').cpus().length;
const numCPUs = 2;

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
  http.createServer(function(request, response) {
    var urlObj = url.parse(request.url, true);
    var value = urlObj.query["value"];
    if (value == ''){
        response.end("no value specified");
    } else {
        response.end("" + value + "");
    }
  }).listen(2000);

  console.log(`Worker ${process.pid} started`);
}