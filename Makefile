INCLUDE=-I./aip-cpp-sdk
LIB=-ljsoncpp -lcurl -lcrypto -lpthread
Ivmt:Ivmt.cc
	g++ -o $@ $^ $(LIB) -std=c++11 $(INCLUDE) #-static
.PHONY:clean
clean:
	rm -f Ivmt
