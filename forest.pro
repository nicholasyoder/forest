TEMPLATE = subdirs

SUBDIRS	= \
    library \
    forest \
    desktop \
    forest-logout \
    panel \
    services \
    session \
    settings \
    system

forest.depends = library
desktop.depends = library
panel.depends = library
settings.depends = library
system.depends = library

include(shared-variables.pri)

INCLUDEPATH += common

#---config files---------

forestConfig.files=etc/forest/*
forestConfig.path=/etc/forest/

sessionConfig.files=etc/xdg/lxsession/forest/*
sessionConfig.path=/etc/xdg/lxsession/forest/

#---DATA files---------------

sessionData.files=usr/share/xsessions/*
sessionData.path=/usr/share/xsessions/

applicationsData.files=usr/share/applications/*
applicationsData.path=/usr/share/applications/

forestData.files=usr/share/forest/*
forestData.path=/usr/share/forest

wallpapersData.files=usr/share/wallpapers/forest/*
wallpapersData.path=/usr/share/wallpapers/forest

themesData.files=usr/share/themes/*
themesData.path=/usr/share/themes

INSTALLS += sessionData \
        forestData\
        wallpapersData\
        themesData\
        applicationsData\
        forestConfig\
        sessionConfig\
