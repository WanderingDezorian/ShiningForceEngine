CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
PROJECT(MINIZIP C CXX)

# Define additional library search paths
SET(ADDL_INC_PATH 
    #<-- Add additional paths here
CACHE PATH "Additional paths to search for included files")
SET(ADDL_LIB_PATH ${CMAKE_LIBRARY_PATH}
    #<-- Add additional paths here
CACHE PATH "Additional paths to search for required libs")


# Define our sources
SET(SRC_FILES
  GraphicsCore.cpp   Logic_Battle.cpp       main.cpp       ResourceCore.cpp
  InterfaceCore.cpp  Logic_StartScreen.cpp  MusicCore.cpp  XmlMap.cpp
  GameState.cpp
)

# Actually use requested additional paths
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${ADDL_INC_PATH} )
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${ADDL_LIB_PATH} )
# Find packages
FIND_PACKAGE ( SDL )
FIND_PACKAGE ( PNG REQUIRED )
FIND_PACKAGE ( ZLIB REQUIRED )
FIND_LIBRARY(VORBISFILE_LIBRARY NAMES vorbisfile_static libvorbisfile_static vorbisfile libvorbisfile PATHS ${ADDL_LIB_PATH})
FIND_LIBRARY(OGG_LIBRARY NAMES ogg_static libogg_static ogg libogg PATHS ${ADDL_LIB_PATH})
FIND_LIBRARY(VORBIS_LIBRARY NAMES vorbis_static libvorbis_static vorbis libvorbis PATHS ${ADDL_LIB_PATH})
FIND_FILE(VORBIS_INCLUDE NAMES vorbis/vorbisfile.h PATHS ${ADDL_INC_PATH} )
ADD_SUBDIRECTORY( MiniZip )
# Workaround for the non-working REQUIRED flag
if ( NOT SDL_FOUND )
   message ( FATAL_ERROR "SDL not found!" )
endif ( NOT SDL_FOUND )

# Define our projects
LINK_LIBRARIES( ${SDL_LIBRARY} ${PNG_LIBRARY} ${VORBISFILE_LIBRARY} ${VORBIS_LIBRARY} ${OGG_LIBRARY} ${ZLIB_LIBRARY} miniunz )
INCLUDE_DIRECTORIES( ${ZLIB_INCLUDE_DIR} ${VORBIS_INCLUDE} ${PNG_PNG_INCLUDE_DIR} ${SDL_INCLUDE_DIR} )

ADD_EXECUTABLE(TestEngine ${SRC_FILES} )

