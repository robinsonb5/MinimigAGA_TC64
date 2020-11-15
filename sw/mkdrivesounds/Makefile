SOUNDS=DiskInsert.raw DiskEject.raw MotorStart.raw MotorLoop.raw MotorStop.raw \
	Step1.raw Step2.raw Step3.raw Step4.raw HDDStep1.raw HDDStep2.raw HDDStep3.raw HDDStep4.raw 

all: drivesnd.bin

drivesnd.bin: mkdrivesounds
	./mkdrivesounds $(SOUNDS) >$@

mkdrivesounds: mkdrivesounds.c
	gcc -o $@ $+


