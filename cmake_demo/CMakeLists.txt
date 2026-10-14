cmake_minimum_required(VERSION 3.16)#这个项目最少需要CMake 3.16

project(sensor_demo C)#项目名 sensor_demo 语言C，不是最终可执行的文件名

add_subdirectory(protocol)
add_subdirectory(driver)
add_subdirectory(app)


