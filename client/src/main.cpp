#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "ocr.grpc.pb.h"

int main() {
    // For cross-device, replace "localhost" with your partner's IP: "192.168.xxx.xxx:50051"
    std::string server_address("localhost:50051");

    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    std::unique_ptr<ocr::OcrService::Stub> stub = ocr::OcrService::NewStub(channel);

    // Build a dummy ImageRequest for now
    ocr::ImageRequest request;
    request.set_batch_id("batch-1");
    request.set_image_id("img-1");
    request.set_image_data("DUMMY_IMAGE_BYTES");  // later: real file bytes

    ocr::ImageResult response;
    grpc::ClientContext context;

    grpc::Status status = stub->ProcessImage(&context, request, &response);

    if (status.ok()) {
        std::cout << "ProcessImage OK\n";
        std::cout << "Batch: " << response.batch_id()
                  << ", Image: " << response.image_id()
                  << "\nText: " << response.text() << std::endl;
    } else {
        std::cerr << "ProcessImage RPC failed: "
                  << status.error_code() << " - "
                  << status.error_message() << std::endl;
    }

    return 0;
}
