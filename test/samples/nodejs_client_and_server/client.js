var net = require('net');


var recvSize = 0;
var recvReplies = 0;
const buf = Buffer.allocUnsafe(2);

var client = new net.Socket();
client.connect(2000, '127.0.0.1');

client.on('connect', function() {
	buf.writeInt8(2, 0);
	buf.writeInt8(1, 1);
	client.write(buf);
});

client.on('data', function(data) {
	++recvReplies;
	if ( ( recvReplies & 0xFFFF ) == 0 )
		console.log( "[" + recvReplies + "] onData(), size = " + data.length );
	recvSize += data.length;
	buf.writeInt8(2, 0);
	buf.writeInt8(((recvReplies%256)-128)|1, 1);
	client.write(buf);
});
