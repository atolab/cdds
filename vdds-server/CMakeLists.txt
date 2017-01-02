cmake_minimum_required(VERSION 3.5)

add_executable(vdds-server server.c)
add_dependencies(vdds-server vdds)
#target_include_directories(vdds-server PUBLIC ../include)
target_link_libraries(vdds-server vdds)

MACRO (CHECK_LIBEVENT)
#    SET(LIBEVENT_FIND_QUIETLY TRUE)

    IF (NOT LIBEVENT_INCLUDE_PATH)
      set(LIBEVENT_INCLUDE_PATH /usr/local/include /opt/local/include)
    ENDIF()

    find_path(LIBEVENT_INCLUDE_DIR event.h PATHS ${LIBEVENT_INCLUDE_PATH})

    if (NOT LIBEVENT_INCLUDE_DIR)
        MESSAGE(SEND_ERROR "Cannot find appropriate event.h in /usr/local/include or /opt/local/include. Use bundled libevent")
    endif() 

    IF (NOT LIBEVENT_LIB_PATHS) 
      set(LIBEVENT_LIB_PATHS /usr/local/lib /opt/local/lib)
    ENDIF()

    find_library(LIBEVENT_LIB event PATHS ${LIBEVENT_LIB_PATHS})
    find_library(LIBEVENT_PTHREADS_LIB event_pthreads PATHS ${LIBEVENT_LIB_PATHS})

    if (NOT LIBEVENT_LIB OR NOT LIBEVENT_PTHREADS_LIB)
        MESSAGE(SEND_ERROR "Cannot find appropriate event lib in /usr/local/lib or /opt/local/lib. Use bundled libevent")
    endif() 

    IF (LIBEVENT_LIB AND LIBEVENT_PTHREADS_LIB AND LIBEVENT_INCLUDE_DIR)
      set(LIBEVENT_FOUND TRUE)
      set(LIBEVENT_LIBS ${LIBEVENT_LIB} ${LIBEVENT_PTHREADS_LIB})
    ELSE()
      set(LIBEVENT_FOUND FALSE)
    ENDIF()

    IF(LIBEVENT_FOUND)
      SET(LIBEVENT_SOURCES "")
      SET(LIBEVENT_LIBRARIES ${LIBEVENT_LIBS})
      SET(LIBEVENT_INCLUDE_DIRS ${LIBEVENT_INCLUDE_DIR})
      SET(LIBEVENT_DEFINES "-DHAVE_LIBEVENT")
    ELSE()
        MESSAGE(SEND_ERROR "Cannot find appropriate system libraries for libevent. Use bundled libevent")
    ENDIF()
ENDMACRO()

CHECK_LIBEVENT()
add_executable(vdds-server2 server2.c)
add_dependencies(vdds-server2 vdds)
#target_include_directories(vdds-server2 PUBLIC ../include)
target_include_directories(vdds-server2 PUBLIC ${LIBEVENT_INCLUDE_DIR})
target_link_libraries(vdds-server2 ${LIBEVENT_LIBRARIES})
target_link_libraries(vdds-server2 vdds)
