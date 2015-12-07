# esp-httpd-docker
A simple http server for the ESP8266

## How to use it
* Only if you use VirtalBox (with boot2docker or docker-machine): first stop the docker VM and then connect your RS232 adapter to the computer. Assuming the adapter is recognised on your machine, go to the VirtualBox configuration screen and and instruct the VM to share your USB adapter. Then start the docker VM again. You can check if your adapter is working fine by running the docker-connect script. If you don't see any stack-traces you should be fine.
* Refer to [esp-open-sdk-docker](https://github.com/nevers/esp-open-sdk-docker) and build the docker container as detailed in the instructions. 
* Provide your wifi details in wifi.h
* Run docker-build to build the development environment image.
* Run docker-flash to make the binaries and flash them to your ESP8266 SoC. 
* After flashing, it will automagically connect to the device and show its output. Grab the ip address from the console and browse to it. By default it will read out the ADC and print its value. Example console output:
'''
[wifi] init
[http] init
[http] listening for GET requests
[wifi] connected
[wifi] got ip: 192.168.1.5
'''

## Hardware setup
For the ESP-12 board, refer to [esp-open-sdk-docker](https://github.com/nevers/esp-open-sdk-docker)
