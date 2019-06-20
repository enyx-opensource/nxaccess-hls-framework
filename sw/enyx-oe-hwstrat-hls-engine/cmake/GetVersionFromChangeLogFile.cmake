# This macro generate version variables
macro(generate_version_variables)
    execute_process(COMMAND sh -c "${CMAKE_SOURCE_DIR}/changelog.sh -v"
        OUTPUT_VARIABLE PACKAGE_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND sh -c "${CMAKE_SOURCE_DIR}/changelog.sh -d"
        OUTPUT_VARIABLE PACKAGE_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    string(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1"
        PACKAGE_VERSION_MAJOR
        ${PACKAGE_VERSION})
    string(REGEX REPLACE "^[0-9]+\\.([0-9]+)\\.[0-9]+" "\\1"
        PACKAGE_VERSION_MINOR
        ${PACKAGE_VERSION})
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1"
        PACKAGE_VERSION_PATCH
        ${PACKAGE_VERSION})
endmacro()
