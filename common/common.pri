DEFINES += COMMON_DEFINED
CONFIG += COMMON_DEFINED

unix:!macx{
    QMAKE_CXXFLAGS += -std=c++11
    exists(/usr/include/eigen3){
        DEFINES += COMMON_WITH_EIGEN
        MODULES += COMMON_WITH_EIGEN
        INCLUDEPATH += -I /usr/include/eigen3
    }
    else{
        MODULES += COMMON_WITHOUT_EIGEN
    }
}

macx{
    CONFIG += c++11
    exists(/libs/include/eigen3){
        DEFINES += COMMON_WITH_EIGEN
        MODULES += COMMON_WITH_EIGEN
        INCLUDEPATH += -I /libs/include/eigen3/
    }
    else{
        MODULES += COMMON_WITHOUT_EIGEN
    }
}

HEADERS += \
    $$PWD/bounding_box.h \
    $$PWD/comparators.h \
    $$PWD/point.h \
    $$PWD/point2d.h \
    $$PWD/serialize.h \
    $$PWD/arrays.h \
    $$PWD/timer.h \
    $$PWD/utils.h \
    $$PWD/hashlib.h \
    $$PWD/file_conversions.h \
    $$PWD/serializable_object.h
