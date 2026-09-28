cmake_minimum_required(VERSION 3.16)#这个项目最少需要CMake 3.16

project(sensor_demo C)#项目名 sensor_demo 语言C，不是最终可执行的文件名

#添加封装sensor库
add_library(sensor
            src/sensor.c)


target_include_directories(sensor PUBLIC
    include)#给sensor_app这个target增加头文件搜索路径include/


#要生成一个叫sensor_app的可执行目标，它由main.c和sensor.c组成
add_executable(sensor_app   #target
    src/main.c
    )

target_link_libraries(sensor_app PRIVATE
    sensor
)