cmake_minimum_required(VERSION 3.6)
project(Fryte_Compiler)

include_directories(include)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
    include/BytecodeIO.h
    include/Class.h
    include/Compiler.h
    include/InstructionGenerator.h
    include/Parser.h
    include/VirtualStack.h
    include/VMTypes.h
    src/BytecodeIO.cpp
    src/Class.cpp
    src/Compiler.cpp
    src/InstructionGenerator.cpp
    src/Parser.cpp
    src/VirtualStack.cpp
    main.cpp)

add_executable(Fryte_Compiler ${SOURCE_FILES})