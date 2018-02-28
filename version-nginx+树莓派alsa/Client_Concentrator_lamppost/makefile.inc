#created by zhangwei on 2017-04-10

INCLUDE_PATH=-I.  -I../carlockandenvparams -I../chargepile -I../env_params -I../lamp -I../linkcheck -I../music -I../global -I../serv -I../common/cfg -I../common/frame -I../common/socket -I../common/log -I/opt/alsa/include
LIB_PATH=-L.  -L../carlockandenvparams -L../chargepile -L../env_params -L../lamp -L../linkcheck -L../music -L../global -L../serv -L../common/cfg -L../common/frame -L../common/socket -L../common/log -L/opt/alsa/lib
LIB=-llockproc -llockandsensors -lenvparamsproc -lglobal -llampproc -llinkchk -lmusicproc -llog -lconfig -lframe -lnetinf -lasound -lpthread -lc -lm -lavformat
CC=gcc
ARCH=ar -rv
