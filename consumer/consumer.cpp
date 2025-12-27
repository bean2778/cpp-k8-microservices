#include "httplib.h"
#include "json.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

using json = nlohmann::json;

// Helper function to get env var with default
std::string getEnv(const char* name, const std::string& defaultValue) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : defaultValue;
}

struct Config {
    int port;
    std::string processorHost;
    std::string processorPort;
    int pollIntervalSeconds;
    std::string processorUrl;
};

Config loadConfig() {
    Config cfg;
    cfg.port = std::stoi(getEnv("PORT", "8082"));
    cfg.processorHost = getEnv("PROCESSOR_HOST", "processor");
    cfg.processorPort = getEnv("PROCESSOR_PORT", "8081");
    cfg.pollIntervalSeconds = std::stoi(getEnv("POLL_INTERVAL_SECONDS", "5"));
    cfg.processorUrl = "http://" + cfg.processorHost + ":" + cfg.processorPort;
    return cfg;
}

int main() {
    std::cout << "Consumer starting..." << std::endl;
    
    // Load configuration
    Config config = loadConfig();
    
    std::cout << "Consumer configuration:" << std::endl;
    std::cout << "  Port: " << config.port << std::endl;
    std::cout << "  Processor URL: " << config.processorUrl << std::endl;
    std::cout << "  Poll interval: " << config.pollIntervalSeconds << "s" << std::endl;
    
    // Background consumption thread
    std::thread consumptionThread([config]() {
        std::cout.flush();
        
        // Give main thread time to start server
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        while (true) {
            std::cout.flush();
            
            try {
                httplib::Client cli(config.processorUrl);
                auto res = cli.Get("/process");
                
                if (res && res->status == 200) {
                    json data = json::parse(res->body);
                    std::cout << "[CONSUME] Original: " << data["original"] 
                              << ", Processed: " << data["processed"] << std::endl;
                    std::cout.flush();
                } else {
                    std::cerr << "[ERROR] Failed to call Processor service" << std::endl;
                    std::cerr.flush();
                }
                
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Consumption error: " << e.what() << std::endl;
                std::cerr.flush();
            }
            
            std::cout.flush();
            
            std::this_thread::sleep_for(
                std::chrono::seconds(config.pollIntervalSeconds)
            );
        }
    });
    
    std::cout.flush();
    
    // HTTP server for manual testing and health checks
    httplib::Server svr;
    
    // Manual consume endpoint
    svr.Get("/consume", [config](const httplib::Request&, httplib::Response& res) {
        std::cout << "[MANUAL] Consume endpoint called" << std::endl;
        std::cout.flush();
        
        httplib::Client cli(config.processorUrl);
        auto processor_res = cli.Get("/process");
        
        if (processor_res && processor_res->status == 200) {
            res.set_content(processor_res->body, "application/json");
            
            json data = json::parse(processor_res->body);
            std::cout << "[MANUAL] Original: " << data["original"] 
                      << ", Processed: " << data["processed"] << std::endl;
            std::cout.flush();
        } else {
            json error;
            error["error"] = "Failed to call Processor service";
            res.status = 500;
            res.set_content(error.dump(), "application/json");
        }
    });
    
    // Health check endpoint
    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json health;
        health["status"] = "healthy";
        health["service"] = "consumer";
        res.set_content(health.dump(), "application/json");
    });
    
    std::cout << "Listening on http://0.0.0.0:" << config.port << std::endl;
    std::cout << "Background consumption running every " << config.pollIntervalSeconds << " seconds" << std::endl;
    std::cout.flush();
    
    // Detach AFTER everything is set up
    consumptionThread.detach();
    
    svr.listen("0.0.0.0", config.port);
    
    return 0;
}