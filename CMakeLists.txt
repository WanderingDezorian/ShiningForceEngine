CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
PROJECT(GAME_ENGINE C CXX)

# Define our sources

SET(SRC_FILES
  GraphicsCore.cpp   Logic_Battle.cpp       main.cpp       ResourceCore.cpp
  InterfaceCore.cpp  Logic_StartScreen.cpp  MusicCore.cpp
)

FIND_PACKAGE ( SDL REQUIRED )
FIND_PACKAGE ( PNG REQUIRED )
FIND_LIBRARY(VORBIS_LIBRARY NAMES vorbisfile libvorbisfile )
# Workaround for the non-working REQUIRED flag
if ( NOT SDL_FOUND )
   message ( FATAL_ERROR "SDL not found!" )
endif ( NOT SDL_FOUND )

# Define our projects
LINK_LIBRARIES( ${SDL_LIBRARY} ${PNG_LIBRARY} ${VORBIS_LIBRARY})

ADD_EXECUTABLE(TestEngine ${SRC_FILES} )

