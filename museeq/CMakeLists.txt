# Project's name
project(museeq)

set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTSCRIPT TRUE)
set(QT_USE_QTUITOOLS TRUE)

# find and setup Qt4 for this project
find_package(Qt4 REQUIRED)

add_definitions(${QT_DEFINITIONS} )

# tell cmake where to search for headers:v
include_directories(.)
include(${QT_USE_FILE})
include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckCXXSourceCompiles)

add_definitions(-DDATADIR="\""${CMAKE_INSTALL_DATADIR}"\"")

if(BINRELOC)
    add_definitions(-DENABLE_BINRELOC=1)
endif()

if(QT_QTSCRIPT_FOUND)
    message(STATUS "Qt Script available")
    add_definitions(-DHAVE_QTSCRIPT=1 )
else()
    message("!!! Qt Script NOT available (scripts won't be available)")
endif()


if(QT_QTUITOOLS_FOUND)
    message(STATUS "Qt UI Tools available")
    add_definitions(-DHAVE_QTUITOOLS=1 )
else()
    message("!!! Qt UI Tools NOT available (incomplete script support, no dialogs)")
endif()

check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(stdio.h HAVE_STDIO_H)
check_include_files(stdarg.h HAVE_STDARG_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stddef.h HAVE_STDDEF_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(errno.h HAVE_ERRNO_H)
check_include_files(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_files(sys/poll.h HAVE_SYS_POLL_H)
check_include_files(sys/epoll.h HAVE_EPOLL_CTL)
check_include_files(sys/signal.h HAVE_SIGNAL_H)
check_include_files(sys/un.h HAVE_SYS_UN_H)
# Check for some standard types and type sizes
check_type_size("unsigned char" UNSIGNED_CHAR)
check_type_size("unsigned short" UNSIGNED_SHORT)
check_type_size("unsigned int" UNSIGNED_INT)
check_type_size("unsigned long" UNSIGNED_LONG)
check_type_size("unsigned long long" UNSIGNED_LONG_LONG)
check_type_size("uint" UINT)
check_type_size("uint32" UINT32)

# Determine the type to use as uint32.
if(HAVE_UINT32)
    set(UINT32_TYPE uint32)
elseif(UNSIGNED_CHAR EQUAL 4)
    set(UINT32_TYPE "unsigned char")
elseif(UNSIGNED_SHORT EQUAL 4)
    set(UINT32_TYPE "unsigned short")
elseif(UNSIGNED_INT EQUAL 4)
    set(UINT32_TYPE "unsigned int")
elseif(UNSIGNED_LONG EQUAL 4)
    set(UINT32_TYPE "unsigned long")
elseif(UNSIGNED_LONG_LONG EQUAL 4)
    set(UINT32_TYPE "unsigned long long")
else()
    message(FATAL_ERROR "Couldn't determine 32bit unsigned integer type.")
endif()


set(CMAKE_REQUIRED_LIBRARIES ${QT_LIBRARIES})
set(CMAKE_REQUIRED_INCLUDES . ${QT_INCLUDE_DIR})

message(STATUS "moc binary:" ${QT_MOC_EXECUTABLE})

set(MUSEEQ_SOURCES
    aclineedit.cpp
    banlist.cpp
    browser.cpp
    browsers.cpp
    buddylist.cpp
    chatpanel.cpp
    chatroom.cpp
    chatrooms.cpp
    chattext.cpp
    chatticker.cpp
    codeccombo.cpp
    iconlistbox.cpp
    ignorelist.cpp
    images.cpp
    interestlist.cpp
    interestlistitem.cpp
    interestlistview.cpp
    interests.cpp
    ipdialog.cpp
    mainwin.cpp
    marquee.cpp
    museekdriver.cpp
    museeq.cpp
    onlinealert.cpp
    prefix.c
    privatechat.cpp
    privatechats.cpp
    publicchat.cpp
    recommendsitem.cpp
    recommendsview.cpp
    roomlist.cpp
    roomlistitem.cpp
    roomlistview.cpp
    script.cpp
    search.cpp
    searches.cpp
    searchfilter.cpp
    searchlistview.cpp
    settingsdialog.cpp
    tabwidget.cpp
    tickerdialog.cpp
    transferlistitem.cpp
    transferlistview.cpp
    transferlistitemprogress.cpp
    transfers.cpp
    trustlist.cpp
    userinfo.cpp
    userinfos.cpp
    userlistitem.cpp
    userlistview.cpp
    usermenu.cpp
    usertabwidget.cpp
    util.cpp
    wishlist.cpp
    wishlistview.cpp
    wishlistitem.cpp
    )

set(MUSEEQ_MOC_HEADERS
    aclineedit.h
    banlist.h
    browser.h
    browsers.h
    buddylist.h
    chatpanel.h
    chatroom.h
    chatrooms.h
    chattext.h
    chatticker.h
    codeccombo.h
    iconlistbox.h
    ignorelist.h
    interestlist.h
    interestlistview.h
    interests.h
    ipdialog.h
    mainwin.h
    marquee.h
    museekdriver.h
    museeq.h
    onlinealert.h
    privatechat.h
    privatechats.h
    publicchat.h
    recommendsview.h
    roomlist.h
    roomlistview.h
    script.h
    search.h
    searches.h
    searchfilter.h
    searchlistview.h
    settingsdialog.h
    tabwidget.h
    tickerdialog.h
    transferlistview.h
    transferlistitemprogress.h
    transfers.h
    trustlist.h
    userinfo.h
    userinfos.h
    userlistview.h
    usermenu.h
    usertabwidget.h
    wishlist.h
    wishlistview.h
    )

add_definitions(-DHAVE_SYSTEM_H)

if(QT_QTNETWORK_FOUND)
    qt4_wrap_cpp( MUSEEQ_SOURCES ${MUSEEQ_MOC_HEADERS} )

    set(TRANSLATIONS
        translations/museeq_de.ts
        translations/museeq_es.ts
        translations/museeq_en.ts
        translations/museeq_fr.ts
        translations/museeq_it.ts
        translations/museeq_nl.ts
        translations/museeq_pl.ts
        translations/museeq_pt_BR.ts
        translations/museeq_ro.ts
        )

    if(RELOAD_TRANSLATIONS)
        QT4_CREATE_TRANSLATION(QM_FILES ${MUSEEQ_SOURCES} translations/museeq_template.ts ${TRANSLATIONS})
    else()
        QT4_ADD_TRANSLATION(QM_FILES ${TRANSLATIONS})
    endif()

    add_executable(museeq ${QM_FILES} ${MUSEEQ_SOURCES} ${MUSEEQ_MOC_HEADERS})

    set(LINK_LIBRARIES Mucipher ${QT_LIBRARIES})

    target_link_libraries(museeq ${LINK_LIBRARIES})

    install(
        TARGETS museeq
        DESTINATION bin
        )

    install(FILES ${QM_FILES}
        DESTINATION ${CMAKE_INSTALL_DATADIR}/museek/museeq/translations)

    install(DIRECTORY themes
        DESTINATION ${CMAKE_INSTALL_DATADIR}/museek/museeq
        PATTERN ".svn" EXCLUDE)

    install(DIRECTORY scripts
        DESTINATION ${CMAKE_INSTALL_DATADIR}/museek/museeq
        PATTERN ".svn" EXCLUDE)

    set(ICONS
        museeq.png
        )

    install(
        FILES ${ICONS}
        DESTINATION "share/pixmaps"
        )
    set(DESKTOP
        museeq.desktop
        )

    install(
        FILES ${DESKTOP}
        DESTINATION "share/applications"
        )
    install(
        FILES "museeq.1"
        DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
        )

    message("--> Museeq-qt4 will be installed")

    if(NOT BINRELOC)
        message("   * without Binary Relocation")
    else()
        message("   * with Binary Relocation")
    endif()

    if(RELOAD_TRANSLATIONS)
        message( "   * rebuilding translations from sources" )
    endif()

else()
    message("!!! museeq will NOT be installed [QTNETWORK not found].")
endif()
