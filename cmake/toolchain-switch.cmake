if(DEFINED ENV{DEVKITPRO})
    set(DEVKITPRO "$ENV{DEVKITPRO}")
else()
    set(DEVKITPRO "/opt/devkitpro")
endif()

if(NOT IS_DIRECTORY "${DEVKITPRO}")
    message(FATAL_ERROR
        "devkitPro was not found at '${DEVKITPRO}'. Install devkitPro with "
        "devkitA64 and libnx, or set the DEVKITPRO environment variable.")
endif()

if(NOT EXISTS "${DEVKITPRO}/cmake/Switch.cmake")
    message(FATAL_ERROR
        "${DEVKITPRO}/cmake/Switch.cmake was not found. Install devkitPro and"
        "the Nintendo Switch development packages.")
endif()

foreach(required_compiler aarch64-none-elf-gcc aarch64-none-elf-g++)
    if(NOT EXISTS "${DEVKITPRO}/devkitA64/bin/${required_compiler}")
        message(FATAL_ERROR
            "${DEVKITPRO}/devkitA64/bin/${required_compiler} was not found. "
            "Install or update devkitA64.")
    endif()
endforeach()

include("${DEVKITPRO}/cmake/Switch.cmake")
