#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "ocr.grpc.pb.h"        
#include "tesseract_wrapper.h" 

#include "thread_pool.h"
#include "tesseract_wrapper.h"
#include "ocr.grpc.pb.h"


class OcrServiceImpl final : public ocr::OcrService::Service {
public:
    explicit OcrServiceImpl(ThreadPool& pool) : pool_(pool) {}

    grpc::Status ProcessImage(grpc::ServerContext* context,
                              const ocr::ImageRequest* request,
                              ocr::ImageResult* response) override {
        std::cout << "Received image: batch=" << request->batch_id()
                  << " id=" << request->image_id()
                  << " size=" << request->image_data().size() << " bytes\n";

        // Copy data we need into local variables for the job
        std::string batchId = request->batch_id();
        std::string imageId = request->image_id();
        std::string data    = request->image_data();

        try {
            // Enqueue OCR job into the pool
            auto fut = pool_.enqueue([data]() {
                return run_tesseract_on_png_bytes(data);
            });

            // Wait for result
            std::string text = fut.get();

            response->set_batch_id(batchId);
            response->set_image_id(imageId);
            response->set_text(text);
            return grpc::Status::OK;
        } catch (const std::exception& ex) {
            std::cerr << "OCR error: " << ex.what() << std::endl;
            return grpc::Status(grpc::StatusCode::INTERNAL, ex.what());
        }
    }

private:
    ThreadPool& pool_;
};


int main() {
    const std::string data_path = "C:/Program Files/Tesseract-OCR/tessdata"; // already working path
    const std::string lang      = "eng";

    if (!init_tesseract(data_path, lang)) {
        std::cerr << "Failed to initialize Tesseract\n";
        return 1;
    }

    ThreadPool pool(std::thread::hardware_concurrency() > 0
                        ? std::thread::hardware_concurrency()
                        : 4);

    std::string server_address("0.0.0.0:50051");
    OcrServiceImpl service(pool);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    auto server = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
    return 0;
}