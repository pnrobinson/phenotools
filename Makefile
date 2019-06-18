


TARGET=phenotools

ALL=${TARGET}


${TARGET}:
	cd src; make
	mv src/phenotools .

.PHONY: clean

clean:
	-rm phenotools
	-rm unittests
	cd src; make clean



