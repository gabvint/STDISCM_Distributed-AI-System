#include <iostream>
#include <memory>

#include <grpcpp/grpcpp.h>
#include "ocr.grpc.pb.h"

int main() {
    // TODO: change this later to your partner's IP (e.g., "192.168.1.23:50051")
    std::string server_address("localhost:50051");

    // Create gRPC channel
    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    std::unique_ptr<ocr::OcrService::Stub> stub = ocr::OcrService::NewStub(channel);

    // Prepare request/response and context
    ocr::Empty request;
    ocr::Pong response;
    grpc::ClientContext context;

    // Call the Ping RPC
    grpc::Status status = stub->Ping(&context, request, &response);

    if (status.ok()) {
        std::cout << "Ping response from server: " << response.message() << std::endl;
    } else {
        std::cerr << "Ping RPC failed: " << status.error_code()
                  << " - " << status.error_message() << std::endl;
    }

    return 0;
}
