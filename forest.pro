TEMPLATE    = subdirs
SUBDIRS	    =\
    desktop \
	forest\
	library \
    forest-panel \
#    forest-desktop \
    forest-services \
    forest-logout \
    forest-session \ 
    forest-settings

INCLUDEPATH += common

#Install

#MKDIR = mkdir -p /etc/forest
#MKDIR = mkdir -p /usr/share/xsessions
#MKDIR = mkdir -p /usr/share/applications
#MKDIR = mkdir -p /usr/share/forest
#MKDIR = mkdir -p /usr/share/wallpapers
#MKDIR = mkdir -p /usr/share/desktop-directories
#MKDIR = mkdir -p /usr/share/icons

#---config files---------

forestConfig.files=etc/forest/*
forestConfig.path=/etc/forest/

sessionConfig.files=etc/xdg/lxsession/forest/*
sessionConfig.path=/etc/xdg/lxsession/forest/

#xfwm4Config.files=etc/xdg/xfce4/xfconf/xfce-perchannel-xml/*
#xfwm4Config.path=/etc/xdg/xfce4/xfconf/xfce-perchannel-xml/

#---DATA files---------------

sessionData.files=usr/share/xsessions/*
sessionData.path=/usr/share/xsessions/

# iconsData.files=usr/share/icons/hicolor/*
# iconsData.path=/usr/share/icons/hicolor/

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
