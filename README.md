# ESP8266-MyWidget-Demo

This project can be used as a learning tool or as a template for starting a new ESP8266 project.
It contains many of the components to build a ESP8266 project.

This demo includes:
- ESP8266 Development
- mDNS - multicast DNS (mywidget.local)
- Asynchronous Web Server (ESPAsyncWebServer) 
- Asynchronous Web Services (ESPAsyncWebServer)
- Asynchronous MQTT (PangolinMQTT) 
- LittleFS File System
- Embedded Bootstrap (3.4.1)
- Embedded jquery (3.5.1)
- Save/Load Config File (text)
- OTA (Over the Air) Updates
- HTTP API Supports HTTP GET/POST
- Auto Updating Web Site (Web Services using JS)
- Interrupt Timer (Flash Onboard LED)
- Plus much more...

## History

I purchased several [ESPixelSticks](https://www.amazon.com/dp/B072XT1V77/ref=cm_sw_em_r_mt_dp_-RscGb7XH5PQ3) from Amazon for my sychronized LED Christmas light display. After using the ESPixelSticks, I became interested in how the ESP8266 worked and how to program it. So I began to review the [ESPixelStick](https://github.com/forkineye/ESPixelStick) source code and the specifications of the ESP8266. It didn't take long to find out there is an add-on for the Arduino IDE that allows you to program the ESP8266 using the Arduino IDE and its programming language. A few years back, I had created some simple projects using Arduino UNO R3 and programmed using the Arduino IDE. Since I was already familar with the Arduino IDE, I started by modifying some of the simple examples and playing with different libraries. I wasn't very interested in the low level hardware or interfacing to a sensor/module but I was more interested in an interactive UI and different ways to communicate with this device. I didn't have a particular project in mind so I created MyWidget; A project template for ESP8266.

MyWidget can be used as a learning tool or as a template for starting a new ESP8266 project. I tried to incorporate many UI and APIs that may be useful for a ESP8266 project with a web interface.

## Requirements

Along with the Arduino IDE, you'll need the following software to build this project:

- [Adruino for ESP8266](https://github.com/esp8266/Arduino) - Arduino core for ESP8266
- [Arduino ESP8266 LittleFS Filesystem Uploader](https://github.com/esp8266/arduino-esp8266fs-plugin) - Arduino plugin for uploading files to LittleFS

The following libraries are required:

Extract the folder in each of these zip files and place it in the "library" folder under your arduino environment

- [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) - Asynchronous TCP Library
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) - Asynchronous HTTP and WebSocket Server for ESP8266 Arduino
- [PangolinMQTT](https://github.com/philbowles/PangolinMQTT) - Asynchronous MQTT client library


## Features

---

## mDNS - multicast DNS

Mulicast DNS allows you to use a local DNS name (mywidget.local) to access the web interface.  There is no need to know the local IP address obtained from the WIFI connection. mDNS will resolve the IP address for you.

Enter the following into browser ``http://mywidget.local``

## Web Services 

- Asynchronous
- Supports GET and SET ACTIONS 
	- Formats: 
    	- cmd:get:action 
    	- cmd:set:action
	- All requests and responses are in TEXT format.
   
  - GET ACTIONS
  	- Uptime - Returns device uptime in milliseconds.
		- Request: ''cmd:get:uptime''
		- Response: ''cmd:get:uptime:NNNNNNNN''
			- where NNNNNNNN is the device uptime in milliseconds
   	- Status - Returns status of light (0/1)
		- Request: ''cmd:get:status''
		- Response: ''cmd:get:status:N''
			- where N is "0" or "1" representing "OFF" or "ON" of light
	- Config - Returns current configuration parameters. Read from /cfg.txt file.
		- Request: ''cmd:get:config''
		- Response: ''cmd:get:config:channels:ports:user1:user2''
			- where channels is NNN, ports is NN, user1 is XXXXX, user2 is XXXXXX
			
  - SET ACTIONS
  	- Toggle - Toggles the current light from 0 to 1 or 1 to 0
		- Request: ''cmd:set:toggle''
		- Response: ''cmd:set:toggle:N''
			- where N is "0" or "1" representing "OFF" or "ON" of light
			
	- Config - Sets current configuration parameters. Saved to /cfg.txt file.
		- Request: ''cmd:set:config:config:channels:ports:user1:user2''
		- Response: ''cmd:set:config:channels:ports:user1:user2''
			- where channels is NNN, ports is NN, user1 is XXXXX, user2 is XXXXXX

## Web Server

- Asynchronous
- Supports serving html, css, js files
- Sets default to index.html
- Serves two embedded web pages (index.html & config.html)
- File not found message

## User Interface

- Nav Bar Menu
- Home and config pages implemented
- Displays Light Output
	- State1 field updates on page load and when the "Toggle" button is pressed
- Displays Device Configuration
	- Channels, ports, user1 and user2 fields update on page load
	- Channels, ports, user1 and user2 are input fields
	- Channels, ports, user1 and user2 can be modified and are saved when "Save Changes" button is pressed
- Displays Device Uptime
	- Uptime is updated on a 1 second basis
	- Uptime format is: "N days, HH:MM:SS"
- All messages to/from config page use Web Services
- Web Services has queue implemented if server is busy
- Bootstrap (3.4.1)
- JQuery (3.5.1)

## HTTP API - GET & POST

- Asynchronous
- HTTP GET HEAP
	- ''http://mywidget.local/heap''

	- Heap
		- Name/Value Pair: N/A
		- Returns:
			- freeHeap=[Free Heap]
	- Example Request:
	- `http://mywidget.local/heap`
	- Example Response:
	- `freeHeap=38616`
	
- HTTP GET STATUS 
- ''http://mywidget.local/status + query string (name/value pairs) in URL'' - Returns status about network, filesystem, signal, heap and chip info in text format. Name value pairs are delimited using colon ":".  Name and value fields are delimited using a equal "=" sign.

- HTTP GET STATUS-JSON 
- ''http://mywidget.local/status-json + query string (name/value pairs) in URL'' - Returns status about network, filesystem, signal, heap and chip info.

	- Network
		- Name/Value Pair: network=true
		- Returns:
			- ssid=[SSID]
			- hostname=[Hostname]
			- ip=[IP Address]
			- gateway=[Gateway]
			- netmask=[Netmask]
		
	- Filesystem
	
		- Name/Value Pair: fs=true
		- Returns:
			- totalBytes=[Total Bytes]
			- usedBytes=[Used Bytes]

	- Signal
	
		- Name/Value Pair: signal=true
		- Returns:
			- strength=[Signal Strength]

	- Heap
	
		- Name/Value Pair: heap=true			
		- Returns:
			- freeHeap=[Free Heap]

	- Chip Info
		- Name/Value Pair: chipInfo=true
	 	- Returns:
			- chipId=[Chip ID]
			- flashChipId=[Flash Chip ID]
			- flashChipSize=[Flash Chip Size]
			- flashChipRealSize=[Flash Chip Real Size]
			
	- Example Request:
	- `http://mywidget.local/status?network=true&fs=true&signal=true&heap=true&chipInfo=true`
	- Example Response (TEXT):
	- `:network=true:ssid=MyWifi:hostname=mywidget:ip=192.168.0.20:gateway=192.168.0.1:netmask=255.255.255.0:signal=true:strength=-68:chipInfo=true:chipId=3016621:flashChipId=1458392:flashChipSize=4194304:flashChipRealSize=4194304:heap=true:freeHeap=38216:fs=true:totalBytes=1024000:usedBytes=327680`

	- Example Request:
	- `http://mywidget.local/status-json?network=true&fs=true&signal=true&heap=true&chipInfo=true`
	- Example Response (JSON):

    	{"network": {  
        "ssid": "MyWifi",  
		"hostname": "mywidget",  
		"ip": "192.168.0.20",  
		"gateway": "192.168.0.1",  
		"netmask": "255.255.255.0"  
		},  
		"signal": {  
		"strength": "-78"  
		},  
		"chipInfo": {  
		"chipId": "3016621",  
		"flashChipId": "1458392",  
		"flashChipSize": "4194304",  
		"flashChipRealSize": "4194304"  
		},  
		"heap": {  
		"freeHeap": "38024"  
		},  
		"fs": {  
		"totalBytes": "1024000",  
		"usedBytes": "327680"  
		}}  

- HTTP POST COUNTER
- ''http://mywidget.local/counter + query string (name/value pairs) in Request Body'' - Sets the internal counter to Request counter value. Range 1 to 9999
	- Name/Value Pair: counter=XXXX
		- Returns:
			- "POST: Counter set to: XXXX"
			
	- Example Request:
	- `POST http://mywidget.local/counter'
	- `form data in Request Body - counter=109`

	- Example Response (TEXT):
	- `POST: Counter set to: 109`

## File System

This project is implmented using the LittleFS file system.

	data (Directory)`
		css (Style Sheet Directory)
			bootstrap.min.css (bootstrap CSS file)
		js (Java Script directory)
			bootstrap.min.js (bootstrap JS file)
			jquery.min.js (jquery JS file)
		cfg.txt (config file - text format)
		index.html (index file - HTML page)
		config.html (config file - HTML page)

If you modify a file locally, you must upload to device using [Arduino ESP8266 LittleFS Filesystem Uploader](https://github.com/esp8266/arduino-esp8266fs-plugin)

The LittleFS implementation for the ESP8266 supports filenames of up to 31 characters + terminating zero (i.e. char filename[32]), and as many subdirectories as space permits. Keep your filenames + directories names short!

## MQTT (Message Queuing Telemetry Transport) - IoT messaging

- Subscribe
	- esp32/get/uptime - Request uptime
		- Response: Publish: esp32/uptime (payload:text:"NNNNNNNN")
			- where NNNNNNNN is the device uptime in milliseconds
				
	- esp32/get/heap - Request current heap size
		- Response: Publish: esp32/heap (payload:text:"NNNNNN")
			- where NNNNNN is the current heap size
				
	- esp32/get/counter - Request current counter value and increment
		- Response: Publish: esp32/counter (payload:text:"NNNNNN")
			- where NNNNNN is the current counter value.
				- Counter is incremented after every esp32/get/counter received. Range 1 to 9999
				
	- esp32/set/counter (payload:"NNNNNN") - Set counter value to payload value (text)
		- Response: Publish: esp32/counter (payload:text:"NNNNNN")
			- where NNNNNN is the current counter value (text). Range 1 to 9999
			
## OTA (Over The Air) Updates

OTA (Over The Air) Updates allows you to update the firmware on the device without being connected via a USB cable.  The updates can occur after the device is connected to WIFI.  The initial firmware installation requires using a USB cable but all subsequent updates can be perform remotely over WIFI.

## Interrupt 

This project has a interrupt timer used to flash the onboard LED.

timer1 is attached to an interrupt handler; ledTimerISR().

timer1 is enabled with the TIM_DIV16 timer divider, TIM_EDGE trigger and TIM_SINGLE for single shot

The 0.5s timer uses a timer1_write multipler of 2,500,000

The calculations for the timer multipler:

80 Mhz / 16 = 5 Mhz

5 Mhz or 0.0000002 uS

0.0000002 uS * 2500000 = 0.5s

## Test & Debug

I used a few different tools to test and debug.  The source code was mainly debugged using Serial.print() messages and the Serial Monitor (Tools->Serial Monitor) in the Arduino IDE.

I also used the Firefox web developer tools (console & network) debug the JavaScript and web services requests / responses.

To test and debug the HTTP GET and POST requests/response messages, I used [POSTMAN](https://www.postman.com/)

To test and debug MQTT, I used Mosquitto and Node-Red.


## Notes

I orginally tried to send / receive JSON messages using the popular Arduino JSON Library [ArduinoJson](https://github.com/bblanchon/ArduinoJson) but I couldn't make it stable.  I kept getting exceptions happening in various places, while stress testing (calling GET heap repeatively), so I eventually removed the ArduinoJson library and references. I converted all Web Services messages to send/receive text messages.  I also converted files to save as text files (cfg.txt) instead of JSON.

Later I went back and added an HTTP GET with JSON response example without any JSON libraries.
 - The Web Services messages text using fields delimited with colons ":".
 - The config files are also text delimited with colons ":".


## References

- [LittleFS](https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html) - Little File System
- [BootStrap](https://getbootstrap.com/docs/3.4/) - BootStrap(3.4.1) - HTML, CSS, and JS framework
- [jQuery](https://jquery.com/) - JS library
- [OTA with Arduino IDE](https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html#arduino-ide) - How to setup and config IDE for OTA updates
- [Random Nerd Tutorials](https://randomnerdtutorials.com/) - Lots of good information on 8266 here

## Credits
 - Lots of great information and the webservices JS script in config.html is from [ESPixelStick](https://github.com/forkineye/ESPixelStick)
