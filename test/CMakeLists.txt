if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

add_executable(test_local_center
  test_local_center.cpp
)

target_link_libraries(test_local_center PUBLIC Boost::boost Boost::unit_test_framework libdak_static)

add_test(NAME test_local_center COMMAND test_local_center)
