TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /usr/local/include
LIBS += -L/usr/local/lib\
     -lopencv_core\
     -lopencv_highgui\
     -lopencv_imgproc\
     -lopencv_imgcodecs\
     -lfreenect2\
     -lopencv_videoio\
        -lopencv_features2d


SOURCES += main.cpp
