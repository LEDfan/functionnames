cmake_minimum_required(VERSION 3.10)
add_definitions("-DDEBUG_ASSERT_DISABLE")

add_subdirectory(cppast)

add_executable(functionnames main.cpp)
target_link_libraries(functionnames PUBLIC cppast)
install(TARGETS functionnames DESTINATION bin)

