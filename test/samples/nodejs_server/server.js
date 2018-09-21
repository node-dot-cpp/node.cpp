var net = require('net');

var recvSize = 1;
var sentSize = 2;
var rqCnt = 3;
var connCnt = 4;

function stats2buffer() {
const buf = Buffer.allocUnsafe(32);
buf.writeInt32LE(recvSize, 0);
buf.writeInt32LE(0, 4);
buf.writeInt32LE(sentSize, 8);
buf.writeInt32LE(0, 12);
buf.writeInt32LE(rqCnt, 16);
buf.writeInt32LE(0, 20);
buf.writeInt32LE(connCnt, 24);
buf.writeInt32LE(0, 28);
return buf;
}


var server = net.createServer(function(socket) {
        socket.on('data', function(data){
		var rqSz = data[1];
		const replyBuf = Buffer.allocUnsafe(rqSz);
                socket.write(replyBuf);
		connCnt++;
		sentSize += rqSz;
		recvSize += data.length;
        });
});
server.listen(2000, '127.0.0.1');

var server1 = net.createServer(function(socket) {
        socket.on('data', function(data){
                socket.write(stats2buffer());
        });
});
server1.listen(2001, '127.0.0.1');
