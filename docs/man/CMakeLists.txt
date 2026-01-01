project("libc8" C)
include(GNUInstallDirs)
install(DIRECTORY man1/ DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
            FILES_MATCHING PATTERN "*.1")
