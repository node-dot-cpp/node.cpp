'use strict';

var net = require('net');
/*
var server = net.createServer(function (socket) {
    socket.write('Echo server\r\n');
    socket.pipe(socket);
});

server.listen(1337, '127.0.0.1');

var net = require('net');

var client = new net.Socket();
client.connect(1337, '127.0.0.1', function () {

    console.log('connect\n');
    var ok = true;
    while (ok) {
        ok = this.write(this.ptr);//, function () { console.log("drain!\n"); });
        this.sentSize += this.ptr.length;
    }
});

client.recvSize = 0;
client.sentSize = 0;
client.size = 64 * 1024;
var buf = Buffer.alloc(client.size);
client.ptr = buf;

client.on('close', function (hadError) { console.log('close\n'); });
client.on('data', function (buffer) {
    console.log('data\n');
    this.recvSize += buffer.length;

    if (this.recvSize >= this.sentSize)
        end();
});

client.on('end', function () { console.log('end\n'); });
client.on('error', function (error) { console.log('error\n'); });
*/




//var server = net.createServer({ allowHalfOpen: true },function (socket) {
var server = net.createServer({ allowHalfOpen: false }, function (socket) {

    socket.on('close', function (hadError) { console.log('close ', hadError, ';'); });
    socket.on('connect', function () {
        console.log('connect;');
    });
    socket.on('data', function (buffer) {
        console.log('data ', buffer.length, ';');
        this.write(buffer);
    });

    socket.on('end', function () { console.log('end;'); /*this.end("goodbye!"); */});
    socket.on('error', function (error) { console.log('error ', error.code, ';'); });
});

server.listen(2000, '127.0.0.1');

var client = new net.Socket();
client.connect(2000, '127.0.0.1', function () {
    console.log('connect;');
    var ok = true;
    while (ok) {
        ok = this.write(this.ptr, function () { console.log("drain;"); });
        this.sentSize += this.ptr.length;
    }
});

client.recvSize = 0;
client.sentSize = 0;
client.ptr = Buffer.alloc(64 * 1024);

client.on('close', function (hadError) { console.log('close', hadError, ';'); this.connect(2000, '127.0.0.1');});
client.on('data', function (buffer) {
    console.log('data', buffer.length, ';');
    this.recvSize += buffer.length;

    if (this.recvSize >= this.sentSize)
        this.end();
});

client.on('end', function () { console.log('end;'); });
client.on('error', function (error) { console.log('error', error.code, ';'); });
