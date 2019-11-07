local iec_104 = require('ckit.lib')

libsocket = require "socket"
libunix = require "socket.unix"
socket = assert(libunix())

SOCKET_FILE = "/tmp/socket"
os.remove(SOCKET_FILE)
assert(socket:bind(SOCKET_FILE))
assert(socket:listen())

iec_104.fetch("meter1.example.com", 2404, SOCKET_FILE, true);
iec_104.fetch("meter2.example.com", 2404, SOCKET_FILE, false);
while true do
    conn = assert(socket:accept())
    data = conn:receive("*a") -- receive all data from socket, until connection is closed
    if (data ~= nil) then
        print("Got data: " .. data)
    else
        print("Got no data!")
    end
end