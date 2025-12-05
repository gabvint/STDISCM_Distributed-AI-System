#include "grpc_client.h"

#include <fstream>
#include <stdexcept>

GrpcClient::GrpcClient(const std::string& serverAddress) {
    auto channel = grpc::CreateChannel(serverAddress, grpc::InsecureChannelCredentials());
    stub_ = ocr::OcrService::NewStub(channel);
}

std::string GrpcClient::readFile(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       std::istreambuf_iterator<char>());
}

GrpcClient::Result GrpcClient::processImage(const std::string& batchId,
                                            const std::string& imageId,
                                            const std::string& filePath) {
    Result result;

    try {
        std::string imageData = readFile(filePath);

        ocr::ImageRequest req;
        req.set_batch_id(batchId);
        req.set_image_id(imageId);
        req.set_image_data(imageData);

        ocr::ImageResult res;
        grpc::ClientContext ctx;

        grpc::Status status = stub_->ProcessImage(&ctx, req, &res);
        if (!status.ok()) {
            result.ok = false;
            result.error = status.error_message();
            return result;
        }

        result.ok = true;
        result.text = res.text();
        return result;
    } catch (const std::exception& ex) {
        result.ok = false;
        result.error = ex.what();
        return result;
    }
}
