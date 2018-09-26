var net = require('net');


var recvSize = 0;
var recvReplies = 0;
const buf = Buffer.allocUnsafe(2);

function castToInt8(x) {
	return (x%256)-128;
}

var clientSock = new net.Socket();
clientSock.connect(2000, '127.0.0.1');

clientSock.on('connect', function() {
	buf.writeInt8(2, 0);
	buf.writeInt8(1, 1);
	clientSock.write(buf);
});

clientSock.on('data', function(data) {
	++recvReplies;
	if ( ( recvReplies & 0xFFFF ) == 0 )
		console.log( "[" + recvReplies + "] onData(), size = " + data.length );
	recvSize += data.length;
	buf.writeInt8(2, 0);
	buf.writeInt8(castToInt8(recvReplies)|1, 1);
	clientSock.write(buf);
});
