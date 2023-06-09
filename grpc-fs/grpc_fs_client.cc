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

            const size_t chunk_size = 1UL << 20;    //20 1MB 30 1GB Hardcoded to 1MB, which seems to be recommended from experience.
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
            // std::cout << "Finished sending file with id " << returnedId.id() << std::endl;
        }

        return true;
    }

    bool DownloadFile(std::int32_t id)
    {
        FileId requestedId;
        FileContent contentPart;
        ClientContext context;
        SequentialFileWriter writer;
        std::string filename;

        requestedId.set_id(id);
        std::unique_ptr<ClientReader<FileContent> > reader(m_stub->DownloadFile(&context, requestedId));
        try {
            while (reader->Read(&contentPart)) {
                assert(contentPart.id() == id);
                filename = contentPart.name();
                writer.OpenIfNecessary(contentPart.name());
                auto* const data = contentPart.mutable_content();
                writer.Write(*data);
            };
            const auto status = reader->Finish();
            if (! status.ok()) {
                std::cerr << "Failed to get the file ";
                if (! filename.empty()) {
                    std::cerr << filename << ' ';
                }
                std::cerr << "with id " << id << ": " << status.error_message() << std::endl;
                return false;
            }
            std::cout << "Finished receiving the file "  << filename << " id: " << id << std::endl;
        }
        catch (const std::system_error& ex) {
            std::cerr << "Failed to receive " << filename << ": " << ex.what();
            return false;
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
    // std::cout << " argv[1] " << argv[1] << " argv[2] " << argv[2] << " argv[3] " << argv[3] << " argv[4] " << argv[4] << std::endl;
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
    
    std::string port(":50051");
    std::string server_address(argv[1]);
    server_address=server_address+port;
    // std::cout << server_address << std::endl;
    FileSystemClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

    if ("put" == verb) {
        if (5 != argc) {
            usage(argv[0]);
        }
        const std::string filename = argv[4];
        for(int i=0;i<1;i++){
            auto timeStart = std::chrono::high_resolution_clock::now();
            succeeded = client.UploadFile(id, filename);
            auto timeEnd = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> diff = timeEnd - timeStart;
            // long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count();
            std::cout<<"Latency: "<<diff.count() << "ms" << std::endl; //duration<<"ms "<< 
        }
    }
    else if ("get" == verb) {
        if (4 != argc) {
            usage(argv[0]);
        }
        auto timeStart = std::chrono::high_resolution_clock::now();
        succeeded = client.DownloadFile(id);
        auto timeEnd = std::chrono::high_resolution_clock::now();

        long long duration = std::chrono::duration_cast<std::chrono::microseconds>(timeEnd - timeStart).count();
        std::cout<<"Latency: "<<duration<<"ms\n";
    }
    else if ("put_all" == verb) {
        if (5 != argc) {
            usage(argv[0]);
        }
        std::int32_t nfiles = atoi(argv[3]);
        const std::string filenamePrefix = argv[4];
        std::string filename;
        for(int i=1; i<=nfiles; i++){
            filename = filenamePrefix + std::to_string(i)+".txt";
            std::cout<<filename<<"\n";
            auto timeStart = std::chrono::high_resolution_clock::now();
            succeeded = client.UploadFile(i, filename);
            auto timeEnd = std::chrono::high_resolution_clock::now();
            
            if(true == succeeded){
                long long duration = std::chrono::duration_cast<std::chrono::microseconds>(timeEnd - timeStart).count();
                std::cout<<"Upload Latency: "<<duration<<"ms\n";
            }
            else
                break;
        }
        
    }
    else if ("get_all" == verb) {
        if (4 != argc) {
            usage(argv[0]);
        }
        std::int32_t nfiles = atoi(argv[3]);
        for(int i=1; i<=nfiles; i++){

            auto timeStart = std::chrono::high_resolution_clock::now();
            succeeded = client.DownloadFile(i);
            auto timeEnd = std::chrono::high_resolution_clock::now();
            if(true == succeeded){
                long long duration = std::chrono::duration_cast<std::chrono::microseconds>(timeEnd - timeStart).count();
                std::cout<<"Latency: "<<duration<<"ms\n";
            }
        }
        
    } 
    else {
        std::cerr << "Unknown verb " << verb << std::endl;
        usage(argv[0]);
    }
    std::cout << "========================" << std::endl;
    return succeeded ? EX_OK : EX_IOERR;
}
