# Works with 3.15 and tested through 4.0
cmake_minimum_required(VERSION 3.15...4.0)

# Project name and a few useful settings. Other commands can pick up the results
project(
  Breakout
  VERSION 0.1
  DESCRIPTION "A simple Breakout game"
  LANGUAGES CXX)

add_subdirectory(apps)
