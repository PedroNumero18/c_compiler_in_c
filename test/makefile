CC = arm-none-eabi-gcc
CFLAGS = -nostdlib -static
TARGET = testChar.s testCMP.s testInt.s testPrintf.s

ALL:$(TARGET)

testChar.s :testChar.c
	$(CC) $(CFLAGS) -S testChar.c -O0 testChar.s

testCMP.s :testCMP.c
	$(CC) $(CFLAGS) -S testCMP.c -O0 testCMP.s

testInt.s :testInt.c
	$(CC) $(CFLAGS) -S testInt.c -O0 testInt.s

testPrintf.s :testPrintf.c
	$(CC) $(CFLAGS) -S testPrintf.c -O0 testPrintf.s

clean:
	rm -rf $(TARGET)