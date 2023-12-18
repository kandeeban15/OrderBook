#include <thread>
#include <algorithm>
#include <sstream>
#include <fstream>
#include "OrderBookManager.h"
#include <iostream>
#include <chrono>

std::mutex mtx_queue;
std::queue<std::unique_ptr<std::string>> messageQueue_print;
std::condition_variable cv_print;
std::atomic<int> count = 0;
std::atomic<int> print_count = 0;
std::chrono::high_resolution_clock::time_point start, end;

void writeMessagesToFile(const std::string& filename) {
    std::ofstream csvFile(filename, std::ios::out | std::ios::app);
    if (!csvFile.is_open()) {
        std::cerr << "Failed to open CSV file for messages." << std::endl;
        return;
    }
    csvFile<<"ClientOrderID,OrderID,Instrument,BuyOrSell,ExecStatus,Quantity,Price"<<std::endl;

    std::unique_lock<std::mutex> lock(mtx_queue);
    while (true) {
        terminator = (count == print_count) && terminator_input;
        if(terminator){
            lock.unlock();
            end = std::chrono::high_resolution_clock::now();
            csvFile<<"End Of Processing"<<std::endl;

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            csvFile<< "Time taken: " << duration.count() << " milliseconds" << std::endl;
            return;
        }
        cv_print.wait(lock, []{ return !messageQueue_print.empty(); });
        while (!messageQueue_print.empty()) {
            auto message = std::move(messageQueue_print.front());
            messageQueue_print.pop(); // Now safe to remove the unique_ptr from the queue
            lock.unlock();
            print_count += 1;
            csvFile << *message << std::flush;
            lock.lock();
        }
    }
}

void processOrders(OrderBook& ob, OrderQueue& queue) {
    OrderData order;
    while (true) {
        if (!queue.isempty()) {
                queue.pop(order);
                auto message = std::make_unique<std::string>(
                        ob.addOrder(order.clientOrderId, order.buyOrSell, order.shares, order.limit, order.sidecorrectness));

                std::lock_guard<std::mutex> lock(mtx_queue);
                messageQueue_print.push(std::move(message));
                cv_print.notify_one();
        }
        else if(terminator){
            return;
        }
    }
}

int main() {
    start = std::chrono::high_resolution_clock::now();
    const std::vector<std::string> instruments = {"Rose", "Lavender", "Lotus", "Tulip", "Orchid"};
    OrderBookManager manager(instruments);

    // Launch threads for each instrument
    std::vector<std::thread> threads;
    for (const auto& instrument : instruments) {
        threads.emplace_back(processOrders, std::ref(manager.getOrderBook(instrument)), std::ref(manager.getOrderQueue(instrument)));
    }
    threads.emplace_back(writeMessagesToFile, "../processed_orders.csv");

    std::ifstream file("../orders.csv");
    if (!file.is_open()) {
        std::cerr << "Failed to open CSV file." << std::endl;
        return 0;
    }

    std::string header;
    if (!std::getline(file, header)) {
        std::cerr << "Failed to read header from CSV file." << std::endl;
        return 0;
    }



    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        OrderData order;
        std::string instrument, buyOrSellString;

        // Read the client order ID first
        std::getline(ss, order.clientOrderId, ',');

        // Then read the instrument name
        std::getline(ss, instrument, ',');

        // Read the buy or sell indicator
        std::getline(ss, buyOrSellString, ',');
        order.buyOrSell = (buyOrSellString == "1");
        order.sidecorrectness = (buyOrSellString == "1" || buyOrSellString == "2");

        // Read the shares and limit
        ss >> order.shares;
        ss.ignore(); // Ignore the comma after shares
        ss >> order.limit;
        auto it = std::find(instruments.begin(), instruments.end(), instrument);
        if(it == instruments.end()){
            messageQueue_print.push(std::make_unique<std::string>("---," + order.clientOrderId + ",InvalidInstrument," + std::to_string(order.buyOrSell) + ",Reject," + std::to_string(order.shares) + "," + std::to_string(order.limit) + "\n"));
        }

        manager.getOrderQueue(instrument).push(order);
        count++;
    }
    terminator_input = true;
    file.close();

    for (auto& thread : threads) {
        if(thread.joinable()){
            thread.join();
        }
    }

    return 1;
}
