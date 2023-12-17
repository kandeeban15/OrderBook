#include "OrderQueue.h"
#include "OrderBook.h"
#include "map"

class OrderBookManager {
    std::map<std::string, OrderBook> orderBooks;
    std::map<std::string, OrderQueue> orderQueues;

public:
    // Constructor that takes a vector of instrument names
    OrderBookManager(const std::vector<std::string>& instruments) {
        for (const auto& instrument : instruments) {
            orderBooks[instrument];
            orderBooks[instrument].instrument = instrument;
            orderQueues[instrument];
        }
    }

    OrderBook& getOrderBook(const std::string& instrument) {
        return orderBooks[instrument];
    }

    OrderQueue& getOrderQueue(const std::string& instrument) {
        return orderQueues[instrument];
    }
};
