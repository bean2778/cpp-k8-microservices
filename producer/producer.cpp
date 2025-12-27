#include "httplib.h"
#include "json.hpp"
#include <random>
#include <iostream>
#include <cstdlib>  // for getenv

using json = nlohmann::json;

// Helper function to get env var with default
std::string getEnv(const char* name, const std::string& defaultValue) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : defaultValue;
}

int main() {
    std::cout << "Producer starting...:" << std::endl;

    httplib::Server svr;

    int port = std::stoi(getEnv("PORT", "8080"));

    svr.Get("/data", [](const httplib::Request& req, httplib::Response& res) {
        // Generate random number
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 100);
        int value = dis(gen);
    
        // Create JSON Response
        json response;
        response["value"] = value;
    
        res.set_content(response.dump(), "application/json");
        std::cout << "Generated: " << value << std::endl;
    });

    std::cout << "Listening on port " << port <<  std::endl;
    svr.listen("0.0.0.0", port);
}