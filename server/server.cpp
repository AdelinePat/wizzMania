#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>

int main()
{
    // Disable buffering entirely
    std::cout.setf(std::ios::unitbuf);

    std::cout << "========================================\n";
    std::cout << " WizzMania Server - Test Bootstrap\n";
    std::cout << "========================================\n";

    const char* port = std::getenv("SERVER_PORT");
    if (!port)
    {
        std::cout << "[WARN] SERVER_PORT not set. Using default 8888\n";
        port = "8888";
    }

    std::cout << "[INFO] Server starting on port: " << port << "\n";
    std::cout << "[INFO] Build & runtime environment OK\n";
    std::cout << "[INFO] Waiting..." << std::endl;

    while (true)
    {
        std::cout << "[HEARTBEAT] Server running...\n";
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
