#created by zhangwei on 2017-04-10

INCLUDE_PATH=-I.  -I../chargepile -I../collector -I../env_params -I../parklock -I../music -I../global -I../muxdemux -I../serv -I../../common/json -I../../common/fcgi -I../../common/cfg -I../../common/frame -I../../common/socket -I../../common/threadpool -I../../common/hash -I../../common/log -I../../common/db -I../../common/md5 -I/usr/local/mysql/include
LIB_PATH=-L.  -L../chargepile -L../collector -L../env_params -L../parklock -L../music -L../global -L../muxdemux -L../../common/json -L../../common/fcgi -L../../common/cfg -L../../common/frame -L../../common/socket -L../../common/log -L../../common/threadpool -L../../common/db -L../../common/md5 -L/usr/local/lib  -L/usr/local/mysql/lib
LIB=-lchargepile -lcollector -lenvir -lparklock -lmusic -lmuxdemux -lglobal -lcJSON -llog -lconfig -lgdmysql -lfcgi -lframe -lnetinf -lthpool -lmysqlclient -lpthread -lmd5 -lc -lm -lz
CC=gcc
ARCH=ar -rv
