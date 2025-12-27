#include "httplib.h"
#include "json.hpp"
#include <iostream>
#include <cstdlib>  // for getenv

using json = nlohmann::json;

// Helper function to get env var with default
std::string getEnv(const char* name, const std::string& defaultValue) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : defaultValue;
}

int main() {
    std::cout << "Producer starting..." << std::endl;

    httplib::Server svr;
    
    // Read configuration from environment
    int port = std::stoi(getEnv("PORT", "8081"));
    std::string producerHost = getEnv("PRODUCER_HOST", "producer");
    std::string producerPort = getEnv("PRODUCER_PORT", "8080");
    std::string producerUrl = "http://" + producerHost + ":" + producerPort;

    svr.Get("/process", [](const httplib::Request&, httplib::Response& res) {
        // Call the producer service
        httplib::Client cli("http://producer:8080");
        auto producer_res = cli.Get("/data");

        if (producer_res && producer_res->status == 200) {
            // Parse the response from Producer
            json producer_data = json::parse(producer_res->body);
            int original_value = producer_data["value"];

            // Process it (multiply by 2)
            int processed_value = original_value * 2;

            std::cout << "Recieved: " << original_value
                        << ", Processed: " << processed_value << std::endl;

            // Return processed result
            json response;
            response["original"] = original_value;
            response["processed"] = processed_value;

            res.set_content(response.dump(), "application/json");
        } else {
            // Error calling Producer
            json error;
            error["error"] = "Failed to call Producer service";
            res.status = 500;
            res.set_content(error.dump(), "application/json");

            std::cerr << "Error: Could not reach Producer" << std::endl;
        }
    });
    
    std::cout << "Processor listening on port " << port << std::endl;
    std::cout << "Producer URL: " << producerUrl << std::endl;
    svr.listen("0.0.0.0", port);
    
    return 0;
}