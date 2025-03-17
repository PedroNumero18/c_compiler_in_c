CC = arm-none-eabi-gcc
TARGET = testChar.s testCMP.s testInt.s testPrintf.s

ALL:$(TARGET)

testChar.s :testChar.c
	$(CC) -S testChar.c -O0 testChar.s

testCMP.s :testCMP.c
	$(CC) -S testCMP.c -O0 testCMP.s

testInt.s :testInt.c
	$(CC) -S testInt.c -O0 testInt.s

testPrintf.s :testPrintf.c
	$(CC) -S testPrintf.c -O0 testPrintf.s

clean:
	rm -rf $(TARGET)