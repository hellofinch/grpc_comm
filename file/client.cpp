#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <utility>
#include <cassert>
#include <sysexits.h>
#include <chrono>

#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

#include "utils.h"
#include "sequential_file_writer.h"
#include "file_reader_into_stream.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using grpcfs::FileId;
using grpcfs::FileContent;
using grpcfs::FileSystem;


class FileSystemClient {
public:
    FileSystemClient(std::shared_ptr<Channel> channel)
        : m_stub(FileSystem::NewStub(channel))
    {
        
    }

    bool UploadFile(std::int32_t id, const std::string& filename)
    {
        FileId returnedId;
        ClientContext context;

        std::unique_ptr<ClientWriter<FileContent>> writer(m_stub->UploadFile(&context, &returnedId));
        try {
            FileReaderIntoStream< ClientWriter<FileContent> > reader(filename, id, *writer);

            const size_t chunk_size = 1UL << 20;    // Hardcoded to 1MB, which seems to be recommended from experience.
            reader.Read(chunk_size);
        }
        catch (const std::exception& ex) {
            std::cerr << "Failed to send the file " << filename << ": " << ex.what() << std::endl;
        }
    
        writer->WritesDone();
        Status status = writer->Finish();
        if (!status.ok()) {
            std::cerr << "File Uploading rpc failed: " << status.error_message() << std::endl;
            return false;
        }
        else {
            std::cout << "Finished sending file with id " << returnedId.id() << std::endl;
        }

        return true;
    }
private:
    std::unique_ptr<grpcfs::FileSystem::Stub> m_stub;
};

void usage [[ noreturn ]] (const char* prog_name)
{
    std::cerr << "USAGE: " << prog_name << "[server address] [put|get|put_all|get_all] num_id [filename]" << std::endl;
    std::exit(EX_USAGE);
}

int main(int argc, char** argv)
{
    if (argc < 4) {
        usage(argv[0]);
    }

    const std::string verb = argv[2];
    std::int32_t id = -1;
    try {
        id = std::atoi(argv[3]);
    }
    catch (std::invalid_argument) {
        std::cerr << "Invalid Id " << argv[3] << std::endl;
        usage(argv[0]);
    }
    bool succeeded = false;
    std::string server_address(argv[1]);
    FileSystemClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

    if ("put" == verb) {
        if (5 != argc) {
            usage(argv[0]);
        }
        const std::string filename = argv[4];
        auto timeStart = std::chrono::high_resolution_clock::now();
        succeeded = client.UploadFile(id, filename);
        auto timeEnd = std::chrono::high_resolution_clock::now();

        long long duration = std::chrono::duration_cast<std::chrono::microseconds>(timeEnd - timeStart).count();
        std::cout<<"Latency: "<<duration<<"ms\n";
    }

    return succeeded ? EX_OK : EX_IOERR;
}
