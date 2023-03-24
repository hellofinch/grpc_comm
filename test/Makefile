CXX = g++
CXXFLAGS += -std=c++14
LDFLAGS += -Wl,--no-as-needed -Wl,--as-needed -libverbs -lrdmacm -g

PROJECT_NAME = test

all: $(PROJECT_NAME)_rdma_client $(PROJECT_NAME)_rdma_server

$(PROJECT_NAME)_rdma_client: client.o
	$(CXX) $^ $(LDFLAGS) -o $@

$(PROJECT_NAME)_rdma_server: server.o
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	rm -f *.o $(PROJECT_NAME)_client $(PROJECT_NAME)_server