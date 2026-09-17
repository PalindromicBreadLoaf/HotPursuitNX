if(DEFINED ENV{DEVKITPRO})
    set(DEVKITPRO "$ENV{DEVKITPRO}")
else()
    set(DEVKITPRO "/opt/devkitpro")
endif()

include("${DEVKITPRO}/cmake/Switch.cmake")
