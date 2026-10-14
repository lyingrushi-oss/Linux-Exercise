
#添加库target
add_library(sensor
            src/sensor.c)

#包含路径
target_include_directories(sensor PUBLIC
                            include) 
target_link_libraries(sensor PRIVATE
                            crc)      