
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(Gomoku)


SET(CMAKE_BUILD_TYPE DEBUG)
SET(CMAKE_CXX_FLAGS_DEBUG "-W -Wall -Wuninitialized -O0 -g3 -O -Winvalid-pch -D_DEBUG")
SET(CMAKE_CXX_FLAGS_RELEASE "-W -Wall -Wuninitialized -O3 -O")

FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtDeclarative REQUIRED)

INCLUDE_DIRECTORIES(
  ${CMAKE_BINARY_DIR}
  board
  Referee
  )

# Rajout des fichiers Qt
INCLUDE(${QT_USE_FILE})

SET(GOMOKU_SRCS
  main.cpp
  ihm/ihm.cpp
  board/Board.cpp
  Referee/Referee.cpp
)

# Headers a passer a Moc
SET( GOMOKU_MOC_HDRS
     ihm/ihm.hpp
     Referee/Referee.hpp
)

# Liste des uis de l'application
SET( GOMOKU_UIS
     ihm/ihm.ui
  )

# Les fichiers ressources
SET( GOMOKU_RCS
  ihm/gomoku.qrc
  )

QT4_ADD_RESOURCES( GOMOKU_RC_SRCS ${GOMOKU_RCS} )
QT4_WRAP_UI( GOMOKU_UI_HDRS ${GOMOKU_UIS} )
QT4_WRAP_CPP( GOMOKU_MOC_SRCS ${GOMOKU_MOC_HDRS} )

ADD_EXECUTABLE(Gomoku ${GOMOKU_SRCS} ${GOMOKU_MOC_SRCS} ${GOMOKU_RC_SRCS} ${GOMOKU_UI_HDRS} )
TARGET_LINK_LIBRARIES(Gomoku ${QT_LIBRARIES})

