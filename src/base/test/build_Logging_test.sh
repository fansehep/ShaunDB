#!/bin/bash


# 编译Logging_test
g++ Logging_test.cpp ../log/Logger.cpp ../log/LogBuffer.cpp ../log/LogFile.cpp ../TimeStamp.cpp ../log/AsyncLoggingThread.cpp -o Logging_test -lfmt -Wall -g -Og -lpthread -std=c++20