FIND_PATH(ZINT_INCLUDE_DIR NAMES zint.h
                           PATHS $ENV{HOME}/local/include
                                 /opt/maiya/local/include)

FIND_LIBRARY(ZINT_LIBRARY NAMES zint
                           PATHS $ENV{HOME}/local/lib
                                 /opt/maiya/local/lib)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Zint DEFAULT_MSG ZINT_LIBRARY ZINT_INCLUDE_DIR)

