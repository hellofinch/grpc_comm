#include <string>

#include <grpc++/grpc++.h>
#include "mathtest.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using mathtest::MathTest;
using mathtest::MathRequest;
using mathtest::MathReply;
using namespace std;

class MathTestClient {
public:
    MathTestClient(std::shared_ptr<Channel> channel) : stub_(MathTest::NewStub(channel)) {}

    int sendRequest(int a, int b) {
        MathRequest request;

        request.set_a(a);
        request.set_b(b);

        MathReply reply;

        ClientContext context;
        // cout << "before send" << std::endl;
        Status status = stub_->sendRequest(&context, request, &reply);

        // cout << "after send" << std::endl;
        if(status.ok()){
            return reply.result();
        }
         else {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return -1;
        }
    }

private:
    std::unique_ptr<MathTest::Stub> stub_;
};

void Run(string argv) {
    std::string port(":5000");
    std::string address=argv+port;
    std::cout << "server ip and port: " << address << std::endl;
    // std::string address("12.9.11.3:5000");
    cout << "before init"<< endl;
    MathTestClient client(
            grpc::CreateChannel(
                    address,
                    grpc::InsecureChannelCredentials()
            )
    );
    cout << "after init" << endl;

    int a = 5;
    int b = 10;

    auto result = client.sendRequest(a, b);
    std::cout << "Answer received: " << a << " * " << b << " = " << result << std::endl;
    result = client.sendRequest(a, b);
    std::cout << "Answer received: " << a << " * " << b << " = " << result << std::endl;
}

int main(int argc, char* argv[]){
    Run(argv[1]);

    return 0;
}
