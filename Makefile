INCLUDE=-I./aip-cpp-sdk
LIB=-ljsoncpp -lcurl -lcrypto -lpthread
Baymax:Baymax.cc
	g++ -o $@ $^ $(LIB) -std=c++11 $(INCLUDE) -D _LOG_
.PHONY:clean
clean:
	rm -f Baymax
