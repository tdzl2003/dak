if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

add_executable(test_logic
  logic_test.cpp
)

target_link_libraries(test_logic PUBLIC Boost::boost Boost::unit_test_framework libdak_static)

add_test(NAME test_logic COMMAND test_logic)
