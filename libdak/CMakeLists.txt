SET(LOCAL_SOURCES_FILES
	../dak/dak.h
	dak_impl.h
	dak.cpp
	local_center.h
    local_center.cpp
	)

ADD_LIBRARY(libdak_static
	STATIC
	${LOCAL_SOURCES_FILES}
)

TARGET_INCLUDE_DIRECTORIES(libdak_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

TARGET_LINK_LIBRARIES(libdak_static PUBLIC Boost::boost Boost::system)