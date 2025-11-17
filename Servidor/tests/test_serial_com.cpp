#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "hardware/SerialCom.h"
#include <iostream>

TEST_SUITE("SerialCom Unit Tests") {
    
    TEST_CASE("Constructor and Getters") {
        SerialCom serial("/dev/ttyTEST", 115200);
        
        CHECK(serial.getPort() == "/dev/ttyTEST");
        CHECK(serial.getBaudrate() == 115200);
        CHECK_FALSE(serial.isDeviceConnected());
    }

    TEST_CASE("Default Constructor Values") {
        SerialCom serial;
        
        CHECK(serial.getPort() == "/dev/ttyUSB0");
        CHECK(serial.getBaudrate() == 19200);
        CHECK_FALSE(serial.isDeviceConnected());
    }

    TEST_CASE("Setters") {
        SerialCom serial;
        
        SUBCASE("Set Port") {
            CHECK(serial.setPort("/dev/ttyUSB0"));
            CHECK(serial.getPort() == "/dev/ttyUSB0");
        }
        
        SUBCASE("Set Baudrate") {
            CHECK(serial.setBaudrate(9600));
            CHECK(serial.getBaudrate() == 9600);
        }
    }

    TEST_CASE("Connection State") {
        SerialCom serial("/dev/nonexistent", 9600);
        
        // Should not be connected initially
        CHECK_FALSE(serial.isDeviceConnected());
        
        // Connect should fail with non-existent port
        CHECK_FALSE(serial.connect());
        
        // Should still not be connected after failed connect
        CHECK_FALSE(serial.isDeviceConnected());
    }

    TEST_CASE("Communication Methods - No Connection") {
        SerialCom serial("/dev/nonexistent", 9600);
        
        SUBCASE("Send Command without connection") {
            CHECK_FALSE(serial.sendCommand("TEST"));
        }
        
        SUBCASE("Read Response without connection") {
            std::string response = serial.readResponse(100);
            CHECK(response.empty());
        }
    }

    TEST_CASE("Copy Operations Deleted") {
        // These should cause compilation errors if uncommented
        // SerialCom serial1;
        // SerialCom serial2 = serial1; // Copy constructor deleted
        // SerialCom serial3;
        // serial3 = serial1; // Copy assignment deleted
        
        CHECK(true); // Just to have at least one assertion
    }
}