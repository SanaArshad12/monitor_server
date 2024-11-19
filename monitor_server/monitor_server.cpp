#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <iomanip> // For output formatting

// Random number generator utility
class RandomGenerator {
private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<> dis;

public:
    RandomGenerator() : gen(rd()), dis(0.0, 1.0) {}

    double generate() {
        return dis(gen);
    }
};

RandomGenerator randomGen; // Global random generator

// Server Class
class Server {
private:
    std::string name;
    double load;
    bool available;

public:
    Server(const std::string& name)
        : name(name), load(randomGen.generate()), available(true) {}

    const std::string& getName() const { return name; }
    double getLoad() const { return load; }
    bool isAvailable() const { return available; }

    void updateStatus() {
        load = randomGen.generate();             // Randomly update load
        available = randomGen.generate() > 0.2; // 80% chance to be available
    }
};

// DashboardController Class
class DashboardController {
public:
    void updateDisplay(const Server& server) {
        std::cout << std::fixed << std::setprecision(2); // Format load to 2 decimal places
        std::cout << "| " << std::setw(10) << server.getName()
            << " | Load: " << std::setw(5) << server.getLoad()
            << " | Available: " << (server.isAvailable() ? "YES" : "NO")
            << " |" << std::endl;
    }

    void showAlert(const std::string& message) {
        std::lock_guard<std::mutex> lock(alertMutex); // Thread-safe alerts
        std::cerr << "\033[1;31mALERT: " << message << "\033[0m" << std::endl; // Red color for alerts
    }

private:
    std::mutex alertMutex; // To synchronize alert messages
};

// ServerMonitor Class
class ServerMonitor {
private:
    std::vector<Server>& servers;
    DashboardController& controller;
    std::atomic<bool> running;
    std::mutex outputMutex;

    void checkForAlerts(Server& server) {
        if (server.getLoad() > 0.9) {
            controller.showAlert("CRITICAL: " + server.getName() + " load exceeds 90%!");
        }
        else if (server.getLoad() > 0.7) {
            controller.showAlert("WARNING: " + server.getName() + " load exceeds 70%.");
        }

        if (!server.isAvailable()) {
            controller.showAlert("ERROR: " + server.getName() + " is unavailable!");
        }
    }

    void monitorLoop() {
        while (running.load()) {
            std::cout << "\n\033[1;34m=== Server Status Update ===\033[0m" << std::endl; // Blue header
            std::cout << "| " << std::setw(10) << "Server"
                << " | " << std::setw(10) << "Details" << " |" << std::endl;
            std::cout << "-------------------------------------" << std::endl;

            for (auto& server : servers) {
                server.updateStatus();
                {
                    std::lock_guard<std::mutex> lock(outputMutex); // Ensure thread-safe output
                    checkForAlerts(server);
                    controller.updateDisplay(server);
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

public:
    ServerMonitor(std::vector<Server>& servers, DashboardController& controller)
        : servers(servers), controller(controller), running(false) {}

    void startMonitoring() {
        running.store(true);
        std::thread(&ServerMonitor::monitorLoop, this).detach(); // Run monitoring in a separate thread
    }

    void stopMonitoring() {
        running.store(false);
    }
};

// Main Function
int main() {
    DashboardController controller;

    std::vector<Server> servers = {
        Server("Server A"),
        Server("Server B"),
        Server("Server C"),
        Server("Server D")
    };

    ServerMonitor monitor(servers, controller);
    monitor.startMonitoring();

    // Run indefinitely (or until manually terminated)
    std::this_thread::sleep_for(std::chrono::hours(1)); // Simulates long-running process

    return 0;
}
