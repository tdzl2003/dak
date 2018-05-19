cmake_minimum_required (VERSION 3.11)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project (dak)

find_package(Boost REQUIRED system thread unit_test_framework exception)

ADD_SUBDIRECTORY(libdak)
ADD_SUBDIRECTORY(test)
