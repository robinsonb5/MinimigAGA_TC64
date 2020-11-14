SOUNDS=DiskInsert.raw DiskEject.raw MotorStart.raw MotorLoop.raw MotorStop.raw Step1.raw Step2.raw Step3.raw Step4.raw HDDClick.raw

all: drivesnd.bin

drivesnd.bin: mkdrivesounds
	./mkdrivesounds $(SOUNDS) >$@

mkdrivesounds: mkdrivesounds.c
	gcc -o $@ $+


