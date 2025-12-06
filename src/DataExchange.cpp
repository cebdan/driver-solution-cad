#include "../include/DataExchange.h"

void DataExchangeInterface::registerDataExchange(DataExchange* exchange) {
    if (exchange) {
        data_exchanges_.push_back(exchange);
    }
}

