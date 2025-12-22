#include "httplib.h"
#include "json.hpp"
#include <random>
#include <iostream>

using json = nlohmann::json;

int main() {
    std::cout << "Producer starting...:" << std::endl;

    httplib::Server svr;

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

    std::cout << "Listening on http://localhost:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);
}