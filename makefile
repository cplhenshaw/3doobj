PROJECT1 = 3doobj
OBJ1 = main1.o modl.o read3do.o checkedMem.o writeObj.o matScaler.o

PROJECT2 = obj3do
OBJ2 = main2.o modl.o read3do.o checkedMem.o objStructs.o readObj.o update3do.o write3do.o matScaler.o

C99 = gcc -std=c99
CFLAGS = -Wall -Werror -pedantic -g

all: $(PROJECT1) $(PROJECT2)

$(PROJECT1) : $(OBJ1)
	$(C99) $(CFLAGS) -o $(PROJECT1) $(OBJ1)

$(PROJECT2) : $(OBJ2)
	$(C99) $(CFLAGS) -o $(PROJECT2) $(OBJ2)

main1.o : modl.h read3do.h writeObj.h main1.c
	$(C99) $(CFLAGS) -c -o main1.o main1.c

main2.o : modl.h read3do.h readObj.h update3do.h write3do.h main2.c
	$(C99) $(CFLAGS) -c -o main2.o main2.c

read3do.o : modl.h checkedMem.h read3do.h read3do.c
	$(C99) $(CFLAGS) -c -o read3do.o read3do.c 

modl.o : modl.h modl.c
	$(C99) $(CFLAGS) -c -o modl.o modl.c

write3do.o : modl.h write3do.h write3do.c
	$(C99) $(CFLAGS) -c -o write3do.o write3do.c

writeObj.o : modl.h writeObj.h writeObj.c
	$(C99) $(CFLAGS) -c -o writeObj.o writeObj.c

checkedMem.o : checkedMem.h checkedMem.c
	$(C99) $(CFLAGS) -c -o checkedMem.o checkedMem.c

objStructs.o : checkedMem.h objStructs.h objStructs.c
	$(C99) $(CFLAGS) -c -o objStructs.o objStructs.c

readObj.o : objStructs.h checkedMem.h readObj.h readObj.c
	$(C99) $(CFLAGS) -c -o readObj.o readObj.c

update3do.o : objStructs.h modl.h checkedMem.h matScaler.h update3do.h update3do.c
	$(C99) $(CFLAGS) -c -o update3do.o update3do.c

matScaler.o : modl.h checkedMem.h matNames.h matSize.h matScaler.h matScaler.c
	$(C99) $(CFLAGS) -c -o matScaler.o matScaler.c

clean:
	rm -f $(OBJ2) $(PROJECT2)
	rm -f $(OBJ1) $(PROJECT1)
	
