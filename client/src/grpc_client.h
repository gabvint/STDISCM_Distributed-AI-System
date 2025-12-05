#pragma once

#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "ocr.grpc.pb.h"

class GrpcClient {
public:
    struct Result {
        bool ok;
        std::string text;
        std::string error;
    };

    explicit GrpcClient(const std::string& serverAddress);

    Result processImage(const std::string& batchId,
                        const std::string& imageId,
                        const std::string& filePath);

private:
    std::unique_ptr<ocr::OcrService::Stub> stub_;

    static std::string readFile(const std::string& path);
};
