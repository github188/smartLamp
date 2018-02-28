#created by zhangwei on 2017-04-10

include ./makefile.inc
CURRENT_DIR=$(shell pwd)

all:
	cd ./common/cfg;\
	make;\
	cd ../frame;\
	make;\
	cd ../socket;\
	make;\
	cd ../log;\
	make;\
	cd $(CURRENT_DIR);\
	cd ./global;\
	make;\
	cd ../carlockandenvparams;\
	make;\
	cd ../chargepile;\
	make;\
	cd ../env_params;\
	make;\
	cd ../lamp;\
	make;\
	cd ../music;\
	make;\
	cd ../linkcheck;\
	make;\
	cd ../serv;\
	make;\

clean:
	cd ./common/cfg;\
	make clean;\
	cd ../frame;\
	make clean;\
	cd ../socket;\
	make clean;\
	cd ../log;\
	make clean;\
	cd $(CURRENT_DIR);\
	cd ./global;\
	make clean;\
	cd ../carlockandenvparams;\
	make clean;\
	cd ../chargepile;\
	make clean;\
	cd ../env_params;\
	make clean;\
	cd ../lamp;\
	make clean;\
	cd ../music;\
	make clean;\
	cd ../linkcheck;\
	make clean;\
	cd ../serv;\
	make clean;\
