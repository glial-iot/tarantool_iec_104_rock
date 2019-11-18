local iec_104 = require('ckit.lib')
local json = require('cjson')

function sleep(n)
    os.execute("sleep " .. tonumber(n))
end

iec_104.meter_add("meter1.example.com", 2404, nil, true)
iec_104.meter_add("meter2.example.com", 2404, nil, false)
local retries = 0
while true do
    data = iec_104.measurement_get()
    if (data ~= nil) then
        print("Got data: " .. data)
        local decoded = json.decode(data)
        local disconnected = decoded.disconnected
        if (disconnected ~= nil and disconnected) then
            print("meter " .. decoded.address .. " disconnected - rearming")
            iec_104.meter_add("meter1.example.com", 2404, nil, true);
        end
    else
        print("Got no data!")
        sleep(1)
    end
    retries = retries + 1
    if (retries == 30) then
        iec_104.meter_remove("meter1.example.com", 2404, nil, true)
    end
    if (retries == 100) then
        iec_104.meter_remove_all()
    end
end
