local iec_104 = require('ckit.lib')
local json = require('cjson')

local socket = require "socket"
local sock = socket.bind('127.0.0.1', 0)
address, port = sock:getsockname()

iec_104.meter_add("meter1.example.com", 2404, port, true)
iec_104.meter_add("meter2.example.com", 2404, port, false)
local retries = 0
while true do
    conn = assert(sock:accept())
    data = conn:receive("*a") -- receive all data from socket, until connection is closed
    if (data ~= nil) then
        print("Got data: " .. data)
        local decoded = json.decode(data)
        local disconnected = decoded.disconnected
        if (disconnected ~= nil and disconnected) then
            print("meter " .. decoded.address .. " disconnected - rearming")
            iec_104.meter_add("meter1.example.com", 2404, port, true);
        end
    else
        print("Got no data!")
    end
    retries = retries + 1
    if (retries == 3) then
        iec_104.meter_remove("meter1.example.com", 2404, port, true)
    end
    if (retries == 10) then
        iec_104.meter_remove_all()
    end
end
