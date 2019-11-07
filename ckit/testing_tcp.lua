local iec_104 = require('ckit.lib')

local socket = require "socket"
local sock = socket.bind('127.0.0.1', 0)
address, port = sock:getsockname()

iec_104.fetch("meter1.example.com", 2404, port, true);
iec_104.fetch("meter2.example.com", 2404, port, false);
while true do
    conn = assert(sock:accept())
    data = conn:receive("*a") -- receive all data from socket, until connection is closed
    if (data ~= nil) then
        print("Got data: " .. data)
    else
        print("Got no data!")
    end
end
