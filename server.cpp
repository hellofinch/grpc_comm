#include <string>

#include <grpc++/grpc++.h>
#include "mathtest.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using mathtest::MathTest;
using mathtest::MathRequest;
using mathtest::MathReply;
using namespace std;

class MathServiceImplementation final : public MathTest::Service {
    Status sendRequest(
            ServerContext* context,
            const MathRequest* request,
            MathReply* reply
    ) override {
        int a = request->a();
        int b = request->b();
        std::cout << "hello" << std::endl;
        // int* x=(int*)malloc(sizeof(int)*128*1024*1024);
        // reply->set_result(x);
        reply->set_result(a * b);

        return Status::OK;
    }
};

void Run(string argv) {
    int a=0;
    // while(a==0){}
    std::string port(":5000");
    std::string address=argv+port;//("12.9.11.3:5000");
    std::cout << "server ip: " << address << std::endl;
    MathServiceImplementation service;
    ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    // cout << "AddListeningPort" << endl;
    builder.RegisterService(&service);
    // cout << "RegisterService" << endl;

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on port: " << address << std::endl;

    server->Wait();
    cout << "Wait" << endl;
}

int main(int argc, char* argv[]) {

    Run(argv[1]);

    return 0;
}
