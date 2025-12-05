#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "ocr.grpc.pb.h"

class OcrServiceImpl final : public ocr::OcrService::Service {
public:
    grpc::Status ProcessImage(grpc::ServerContext* context,
                              const ocr::ImageRequest* request,
                              ocr::ImageResult* response) override {
        std::cout << "Received image: batch=" << request->batch_id()
                  << " id=" << request->image_id()
                  << " size=" << request->image_data().size() << " bytes\n";

        // TODO: later: call Tesseract here.
        std::string fakeText = "[dummy OCR text for " + request->image_id() + "]";

        response->set_batch_id(request->batch_id());
        response->set_image_id(request->image_id());
        response->set_text(fakeText);

        return grpc::Status::OK;
    }
};

int main() {
    std::string server_address("0.0.0.0:50051");

    OcrServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
    return 0;
}
