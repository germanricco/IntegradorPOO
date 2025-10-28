#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../include/File.h"
#include <filesystem>
#include <fstream>

TEST_SUITE("File Class - Essential Operations") {
    
    struct TestSetup {
        TestSetup() { 
            std::filesystem::create_directories("test_data"); 
        }
        ~TestSetup() { 
            std::filesystem::remove_all("test_data"); 
        }
    };

    TEST_CASE_FIXTURE(TestSetup, "File Creation and Basic Operations") {
        File file("test.gcode", "test_data");
        
        SUBCASE("Write and read") {
            file.open(FileMode::WRITE);
            file.write("G28\r\nG1 X100 Y50\r\n");
            file.close();
            
            file.open(FileMode::READ);
            std::string content = file.readAll();
            CHECK(content == "G28\r\nG1 X100 Y50\r\n");
        }

        SUBCASE("Append operations") {
            // Escribir contenido inicial
            file.open(FileMode::WRITE);
            file.write("G28\r\n");
            file.close();
            
            // Append contenido adicional
            file.open(FileMode::APPEND);
            file.append("G1 X100 Y50\r\n");
            file.close();
            
            // Leer y verificar
            auto lines = file.readLines();
            REQUIRE(lines.size() == 2);
            CHECK(lines[0] == "G28");
            CHECK(lines[1] == "G1 X100 Y50");
        }

        SUBCASE("Direct write without explicit open") {
            file.write("Line 1\r\n");
            file.close();
            
            file.write("Line 2\r\n");
            file.close();
            
            auto lines = file.readLines();
            // El segundo write sobreescribe el archivo
            REQUIRE(lines.size() == 1);
            CHECK(lines[0] == "Line 2");
        }
    }

    TEST_CASE_FIXTURE(TestSetup, "GCode Integration") {
        File gcodeFile("robot_commands.gcode", "test_data");
        
        SUBCASE("Write GCode commands") {
            gcodeFile.open(FileMode::WRITE);
            gcodeFile.write("G28\r\n");
            gcodeFile.write("G90\r\n"); 
            gcodeFile.write("G1 X100 Y50 Z20 F300\r\n");
            gcodeFile.close();
            
            // Verificar formato GCode correcto
            gcodeFile.open(FileMode::READ);
            std::string content = gcodeFile.readAll();
            
            CHECK(content.find("G28") != std::string::npos);
            CHECK(content.find("G90") != std::string::npos);
            CHECK(content.find("G1 X100 Y50 Z20 F300") != std::string::npos);
        }

        SUBCASE("Append GCode commands") {
            gcodeFile.open(FileMode::WRITE);
            gcodeFile.write("G28\r\n");
            gcodeFile.close();
            
            gcodeFile.open(FileMode::APPEND);
            gcodeFile.append("G1 X50 Y25 Z10 F200\r\n");
            gcodeFile.close();
            
            auto lines = gcodeFile.readLines();
            REQUIRE(lines.size() == 2);
            CHECK(lines[0] == "G28");
            CHECK(lines[1] == "G1 X50 Y25 Z10 F200");
        }
    }

    TEST_CASE_FIXTURE(TestSetup, "File Properties and Utilities") {
        File file("properties_test.txt", "test_data");
        
        SUBCASE("File properties") {
            CHECK(file.getFilename() == "properties_test.txt");
            CHECK(file.getDirectory().find("test_data") != std::string::npos);
            CHECK(file.getFilePath().find("test_data/properties_test.txt") != std::string::npos);
            
            CHECK_FALSE(file.exists());
            CHECK(file.isEmpty());
            
            file.open(FileMode::WRITE);
            file.write("Test content");
            file.close();
            
            CHECK(file.exists());
            CHECK_FALSE(file.isEmpty());
            CHECK(file.getSize() == 12);
        }

        SUBCASE("Empty file operations") {
            File emptyFile("empty.txt", "test_data");
            emptyFile.open(FileMode::WRITE);
            emptyFile.close();
            
            CHECK(emptyFile.isEmpty());
            CHECK(emptyFile.exists());
            
            emptyFile.open(FileMode::READ);
            std::string content = emptyFile.readAll();
            CHECK(content.empty());
            
            auto lines = emptyFile.readLines();
            CHECK(lines.empty());
        }
    }

    TEST_CASE_FIXTURE(TestSetup, "Automatic Directory Creation") {
        SUBCASE("Nested directories") {
            // El directorio se crea en el constructor, así que ya existe
            File nestedFile("test.txt", "test_data/nested/deep/directory/");
            
            // Verificar que el archivo no existe aún
            CHECK_FALSE(nestedFile.exists());
            
            nestedFile.open(FileMode::WRITE);
            nestedFile.write("Auto-created directory!");
            nestedFile.close();
            
            CHECK(nestedFile.exists());
        }
    }
}

TEST_SUITE("File Class - Edge Cases") {
    
    struct TestSetup {
        TestSetup() { 
            std::filesystem::create_directories("test_data"); 
        }
        ~TestSetup() { 
            std::filesystem::remove_all("test_data"); 
        }
    };

    TEST_CASE_FIXTURE(TestSetup, "Special Characters and Formatting") {
        File file("special_chars.gcode", "test_data");
        
        SUBCASE("GCode with comments") {
            file.open(FileMode::WRITE);
            file.write("G28 ; Homing command\r\n");
            file.write("G90 ; Absolute positioning\r\n");
            file.write("G1 X100 Y50 Z20 F300 ; Move to position\r\n");
            file.close();
            
            auto lines = file.readLines();
            REQUIRE(lines.size() == 3);
            CHECK(lines[0] == "G28 ; Homing command");
            CHECK(lines[1] == "G90 ; Absolute positioning");
            CHECK(lines[2] == "G1 X100 Y50 Z20 F300 ; Move to position");
        }

        SUBCASE("Multiple consecutive newlines") {
            file.open(FileMode::WRITE);
            file.write("G28\r\n\r\nG1 X100 Y50\r\n");
            file.close();
            
            auto lines = file.readLines();
            REQUIRE(lines.size() == 3);
            CHECK(lines[0] == "G28");
            CHECK(lines[1] == "");
            CHECK(lines[2] == "G1 X100 Y50");
        }
    }

    TEST_CASE_FIXTURE(TestSetup, "Error Handling") {
        SUBCASE("Read non-existent file") {
            File file("nonexistent.txt", "test_data");
            
            // readAll debería lanzar excepción si el archivo no existe
            CHECK_THROWS_AS(file.readAll(), std::runtime_error);
        }
        
        SUBCASE("Operations on closed file") {
            File file("test.txt", "test_data");
            file.open(FileMode::WRITE);
            file.write("test");
            file.close();
            
            // Escribir en archivo cerrado debería reabrirse automáticamente
            CHECK_NOTHROW(file.write("more data"));
        }
    }
}