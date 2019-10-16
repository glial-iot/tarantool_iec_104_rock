**Installation:**  

Copy the `./iec104-scm-1.rockspec` to tarantool/glial foder.  
Run `tarantoolctl rocks install iec104-scm-1.rockspec`.  

**Features:**  

Gets values of all IOA present on IEC-104 slave (server) device and returns them in JSON format.

**Dependencies:**  

JSON-C Library `https://github.com/json-c/json-c`

**Quick install in Glial Docker container**
1. Download and run Glial container  
`docker pull glial/glial`  
`docker run -p 8888:8080 -td glial/glial`  
2. Inside container, run:  
`apt-get install wget`  
`apt-get install libmsgpuck-dev`  
`apt-get install libjson-c-dev  `  
`wget https://raw.githubusercontent.com/glial-iot/tarantool_iec_104_rock/master/iec104-scm-1.rockspec`  
`tarantoolctl rocks install iec104-scm-1.rockspec` 