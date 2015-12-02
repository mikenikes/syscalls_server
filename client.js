var PORT = 15555;
var HOST = '127.0.0.1';

var dgram = require('dgram');

var recv = dgram.createSocket('udp4');

recv.on("listening", function () {
  var address = recv.address();
  console.log("server listening " +
      address.address + ":" + address.port);
});

recv.on('message', function(msg, rinfo) {
  console.log('Received %d bytes from %s:%d\n',
              msg.length, rinfo.address, rinfo.port);
  console.log(msg.toString());
});

recv.bind(PORT, function() {
});