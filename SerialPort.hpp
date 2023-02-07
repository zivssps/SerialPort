#pragma once

#define ARDUINO_WAIT_TIME 2000
#define MAX_DATA_LENGTH 1024

#include <windows.h>
#include <iostream>
#include <string>

class SerialPort {
private:
    std::string readBuffer;
    HANDLE handler;
    bool connected;
    COMSTAT status;
    DWORD errors;
public:
    explicit SerialPort(const char* portName, DWORD baudRate = CBR_9600);
    ~SerialPort();
    int readSerialPort(const char* buffer, unsigned int buf_size);
    int readSerialPortUntil(std::string* payload, std::string until);
    bool writeSerialPort(const char* buffer, unsigned int buf_size);
    bool writeSerialPort(std::string payload);
    bool isConnected();
    void closeSerial();
};