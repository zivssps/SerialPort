#include <iostream>
#include <string>
#include "SerialPort.hpp"

SerialPort::SerialPort(const char* portName, DWORD baudRate) {
    this->connected = false;
    this->handler = CreateFileA(static_cast<LPCSTR>(portName),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (this->handler == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            std::cerr << "ERROR: Handle was not attached.Reason : " << portName << " not available\n";
        } else {
            std::cerr << "ERROR!!!\n";
        }
    } else {
        DCB dcbSerialParameters = { 0 };
        if (!GetCommState(this->handler, &dcbSerialParameters)) {
            std::cerr << "ERROR: Failed to get current serial parameters\n";
        } else {
            dcbSerialParameters.BaudRate = baudRate;
            dcbSerialParameters.ByteSize = 8;
            dcbSerialParameters.StopBits = ONESTOPBIT;
            dcbSerialParameters.Parity = NOPARITY;
            dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;
            if (!SetCommState(handler, &dcbSerialParameters)) {
                std::cout << "ALERT: could not set serial port parameters\n";
            } else {
                this->connected = true;
                PurgeComm(this->handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
                Sleep(ARDUINO_WAIT_TIME);
            }
        }
    }
}

SerialPort::~SerialPort() {
    if (this->connected) {
        this->connected = false;
        CloseHandle(this->handler);
    }
}

// Reading bytes from serial port to buffer;
// returns read bytes count, or if error occurs, returns 0

int SerialPort::readSerialPort(const char* buffer, unsigned int buf_size) {
    DWORD bytesRead{};
    unsigned int toRead = 0;
    ClearCommError(this->handler, &this->errors, &this->status);
    if (this->status.cbInQue > 0) {
        if (this->status.cbInQue > buf_size) {
            toRead = buf_size;
        } else {
            toRead = this->status.cbInQue;
        }
    }
    memset((void*)buffer, 0, buf_size);
    if (ReadFile(this->handler, (void*)buffer, toRead, &bytesRead, NULL)) {
        return bytesRead;
    }
    return 0;
}

int SerialPort::readSerialPortUntil(std::string* payload, std::string until) {
    int untilLen = until.length(), len;
    do {
        char buffer[MAX_DATA_LENGTH];
        int bytesRead = this->readSerialPort(buffer, MAX_DATA_LENGTH);
        if (bytesRead == 0) {
            int endIndex = this->readBuffer.find(until);
            if (endIndex != -1) {
                int index = endIndex + untilLen;
                *payload = this->readBuffer.substr(0, index);
                this->readBuffer = this->readBuffer.substr(index);
            }
            return bytesRead;
        }
        this->readBuffer += std::string(buffer);
        len = this->readBuffer.length();
    } while (this->readBuffer.find(until) == std::string::npos);
    int index = this->readBuffer.find(until) + untilLen;
    *payload = this->readBuffer.substr(0, index);
    this->readBuffer = this->readBuffer.substr(index);
    return index;
}

// Sending provided buffer to serial port;
// returns true if succeed, false if not
bool SerialPort::writeSerialPort(const char* buffer, unsigned int buf_size) {
    DWORD bytesSend;
    if (!WriteFile(this->handler, (void*)buffer, buf_size, &bytesSend, 0)) {
        ClearCommError(this->handler, &this->errors, &this->status);
        return false;
    }
    return true;
}

bool SerialPort::writeSerialPort(std::string payload) {
    return this->writeSerialPort(payload.c_str(), payload.length());
}

// Checking if serial port is connected
bool SerialPort::isConnected() {
    if (!ClearCommError(this->handler, &this->errors, &this->status)) {
        this->connected = false;
    }
    return this->connected;
}

void SerialPort::closeSerial() {
    CloseHandle(this->handler);
}