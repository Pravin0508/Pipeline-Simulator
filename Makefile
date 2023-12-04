CC=g++
CFLAGS=-std=c++11
all: sample

compile: 5stage.cpp
	$(CC) $(CFLAGS) 5stage.cpp -o 5stage
	$(CC) $(CFLAGS) 5stage_bypass.cpp -o 5stage_bypass
	$(CC) $(CFLAGS) 79stage.cpp -o 79stage
	$(CC) $(CFLAGS) 79stage_bypass.cpp -o 79stage_bypass
run_5stage:
	./5stage

run_5stage_bypass:
	./5stage_bypass

run_79stage:
	./79stage
	
run_79stage_bypass:
	./79stage_bypass

clean:
	rm -f 5stage 5stage_bypass 79stage 79stage_bypass
