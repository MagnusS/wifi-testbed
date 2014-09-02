all: src/aggie/aggie

src/aggie/aggie:
	cd src/aggie && $(MAKE) 

clean:
	cd src/aggie && make clean
