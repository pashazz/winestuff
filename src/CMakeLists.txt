cmake_minimum_required(VERSION 2.6)
if (${CMAKE_SYSTEM_NAME} MATCHES Linux)
  message (STATUS "Building Winestuff on Linux")
else (${CMAKE_SYSTEM_NAME} MATCHES Linux)
  message (FATAL ERROR "Only Linux is supported by Winestuff")
endif (${CMAKE_SYSTEM_NAME} MATCHES Linux)

option (WISOTOOL "Build WisoTool experimental plug-in" OFF)

find_package( Qt4 REQUIRED )
set(QT_USE_QTGUI 0)
            IF(QT_QTNETWORK_FOUND)
                SET(QT_USE_QTNETWORK 1)
            ELSE(QT_QTNETWORK_FOUND)
		MESSAGE(FATAL ERROR "Qt Network libraries are missing.")
            ENDIF(QT_QTNETWORK_FOUND)

            IF(QT_QTCORE_FOUND)
                SET(QT_USE_QTCORE 1)
            ELSE(QT_QTCORE_FOUND)
		MESSAGE(FATAL ERROR "Qt Core libraries are missing.")
            ENDIF(QT_QTCORE_FOUND)

           IF(QT_QTSQL_FOUND)
                SET(QT_USE_QTSQL 1)
            ELSE(QT_QTSQL_FOUND)
		MESSAGE(FATAL ERROR "Qt SQL libraries are missing.")
            ENDIF(QT_QTSQL_FOUND)

set (CMAKE_BUILD_TYPE Debug)
include (${QT_USE_FILE})
set (WSTDIR ${CMAKE_CURRENT_SOURCE_DIR})
# Compile Native plug-in
add_subdirectory(plugins/native)


if (WISOTOOL MATCHES ON)
  #compile wisotool plug-in
  add_subdirectory(plugins/wisotool)
endif (WISOTOOL MATCHES ON)
set (QHEADERS corelib.h  dvdrunner.h  poldownloader.h  prefix.h sourcereader.h  uiclient.h prefixcollection.h plugincore.h)
set (HEADERS ${QHEADERS}  formatinterface.h libwinegame_global.h)
set (SOURCES corelib.cpp  dvdrunner.cpp  poldownloader.cpp  prefix.cpp sourcereader.cpp prefixcollection.cpp plugincore.cpp)
message (STATUS "Run 'make translations && make' to compile winestuff")

qt4_wrap_cpp (MOC_SOURCES ${QHEADERS})

#Translations
set (TSDIR ${CMAKE_CURRENT_SOURCE_DIR}/l10n)
set (LANGUAGES lwg_ru)

foreach (LANGUAGE ${LANGUAGES})
set (TS ${TSDIR}/${LANGUAGE}.ts)
set (QM ${TSDIR}/${LANGUAGE}.qm)
set (TRANSLATIONS ${TRANSLATIONS} ${TS})
set (TRANSLATIONS_BINARY ${TRANSLATIONS_BINARY} ${QM})
add_custom_command (
OUTPUT ${QM}
COMMAND ${QT_LRELEASE_EXECUTABLE} ${TS}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/l10n
MAIN_DEPENDENCY ${TS})
endforeach()
add_custom_target (
translations
COMMAND ${QT_LUPDATE_EXECUTABLE} ${HEADERS} ${SOURCES} -ts ${TRANSLATIONS}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command (
TARGET translations
COMMAND ${QT_LRELEASE_EXECUTABLE} ${TRANSLATIONS}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_library(winestuff SHARED ${HEADERS} ${SOURCES} ${MOC_SOURCES} )
target_link_libraries( winestuff ${QT_LIBRARIES} )
set(LIB_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/)
set(LIB_INCLUDE_PATH ${CMAKE_INSTALL_PREFIX}/include/winestuff)

install (FILES ${HEADERS} DESTINATION ${LIB_INCLUDE_PATH})
install (TARGETS winestuff DESTINATION ${LIB_INSTALL_PATH})
install (FILES FindWineStuff.cmake DESTINATION ${CMAKE_ROOT}/Modules)
install (FILES ${TRANSLATIONS_BINARY} DESTINATION share/winestuff/l10n)
