#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "ocr.grpc.pb.h"

class OcrServiceImpl final : public ocr::OcrService::Service {
public:
    grpc::Status Ping(grpc::ServerContext* context,
                      const ocr::Empty* request,
                      ocr::Pong* response) override {
        std::cout << "Ping received from client." << std::endl;
        response->set_message("Hello from server!");
        return grpc::Status::OK;
    }
};

int main() {
    std::string server_address("0.0.0.0:50051"); // listen on all interfaces, port 50051

    OcrServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait(); // block and wait for requests
    return 0;
}
