cmake_minimum_required(VERSION 2.6)
project(winestuff)
if (${CMAKE_SYSTEM_NAME} MATCHES Linux)
  message (STATUS "Building WineGame on Linux")
else (${CMAKE_SYSTEM_NAME} MATCHES Linux)
  message (FATAL ERROR "Only Linux is supported by WineGame")
endif (${CMAKE_SYSTEM_NAME} MATCHES Linux)

find_package( Qt4 REQUIRED )
            IF(QT_QTNETWORK_FOUND)
                SET(QT_USE_QTNETWORK 1)
            ELSE(QT_QTNETWORK_FOUND)
                MESSAGE(FATAL ERROR "Qt NetWork libraries missing.")
            ENDIF(QT_QTNETWORK_FOUND)

            IF(QT_QTCORE_FOUND)
                SET(QT_USE_QTCORE 1)
            ELSE(QT_QTCORE_FOUND)
                MESSAGE(FATAL ERROR "Qt Core libraries missing.")
            ENDIF(QT_QTCORE_FOUND)

           IF(QT_QTSQL_FOUND)
                SET(QT_USE_QTSQL 1)
            ELSE(QT_QTSQL_FOUND)
                MESSAGE(FATAL ERROR "Qt SQL libraries missing.")
            ENDIF(QT_QTSQL_FOUND)

include (${QT_USE_FILE})
set (QHEADERS corelib.h  dvdrunner.h  poldownloader.h  prefix.h  uiclient.h)
set (HEADERS ${QHEADERS} libwinegame_global.h)
set (SOURCES corelib.cpp  dvdrunner.cpp  poldownloader.cpp  prefix.cpp)
message (STATUS "Run 'make translations && make' to compile winestuff")

qt4_wrap_cpp (MOC_SOURCES ${QHEADERS})

#Translations
set (TSDIR ${CMAKE_BINARY_DIR}/l10n)
set (LANGUAGES lwg_ru)

foreach (LANGUAGE ${LANGUAGES})
	set (TS ${TSDIR}/${LANGUAGE}.ts)
	set (QM ${TSDIR}/${LANGUAGE}.qm)
	set (TRANSLATIONS ${TRANSLATIONS} ${TS})
	set (TRANSLATIONS_BINARY ${TRANSLATIONS_BINARY} ${QM})
	add_custom_command (
		OUTPUT ${QM}
		COMMAND ${QT_LRELEASE_EXECUTABLE} ${TS}
		MAIN_DEPENDENCY ${TS})
endforeach()
add_custom_target (
	translations 
	COMMAND ${QT_LUPDATE_EXECUTABLE} ${HEADERS} ${SOURCES} -ts ${TRANSLATIONS})
add_custom_command (
	TARGET translations
	COMMAND ${QT_LRELEASE_EXECUTABLE} ${TRANSLATIONS})


add_library(winestuff SHARED ${HEADERS} ${SOURCES} ${MOC_SOURCES} )

target_link_libraries( winestuff ${QT_LIBRARIES} )
set(LIB_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/)
set(LIB_INCLUDE_PATH ${CMAKE_INSTALL_PREFIX}/include/winestuff)

install (FILES ${HEADERS} DESTINATION ${LIB_INCLUDE_PATH})
install (TARGETS winestuff DESTINATION ${LIB_INSTALL_PATH})
install (FILES FindWineStuff.cmake DESTINATION ${CMAKE_ROOT}/Modules)
install (FILES ${TRANSLATIONS_BINARY} DESTINATION share/winestuff/l10n)
