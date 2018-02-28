#created by zhangwei on 2017-04-10

include ./makefile.inc
CURRENT_DIR=$(shell pwd)

all:
	cd ../common/cfg;\
	make;\
	cd ../fcgi;\
	make;\
	cd ../frame;\
	make;\
	cd ../socket;\
	make;\
	cd ../json;\
	make;\
	cd ../log;\
	make;\
	cd ../db;\
	make;\
	cd ../threadpool;\
	make;\
	cd ../md5;\
	make;\
	cd $(CURRENT_DIR);\
	cd ./global;\
	make;\
	cd ../muxdemux;\
	make;\
	cd ../chargepile;\
	make;\
	cd ../collector;\
	make;\
	cd ../env_params;\
	make;\
	cd ../parklock;\
	make;\
	cd ../music;\
	make;\
	cd ../serv;\
	make;\

clean:
	cd ../common/cfg;\
	make clean;\
	cd ../fcgi;\
	make clean;\
	cd ../frame;\
	make clean;\
	cd ../socket;\
	make clean;\
	cd ../json;\
	make clean;\
	cd ../log;\
	make clean;\
	cd ../db;\
	make clean;\
	cd ../threadpool;\
	make clean;\
	cd ../md5;\
	make clean;\
	cd $(CURRENT_DIR);\
	cd ./global;\
	make clean;\
	cd ../muxdemux;\
	make clean;\
	cd ../chargepile;\
	make clean;\
	cd ../collector;\
	make clean;\
	cd ../env_params;\
	make clean;\
	cd ../parklock;\
	make clean;\
	cd ../music;\
	make clean;\
	cd ../serv;\
	make clean;\
