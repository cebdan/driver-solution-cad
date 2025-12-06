#ifndef DATA_EXCHANGE_H
#define DATA_EXCHANGE_H

#include <string>
#include <vector>
#include <memory>

class Solution;

class DataExchange {
public:
    virtual ~DataExchange() = default;
    
    virtual bool canProcess(const std::string& data_type) const = 0;
    virtual void* processData(void* data, const std::string& data_type) = 0;
    virtual std::vector<std::string> getSupportedDataTypes() const = 0;
    
    virtual void setSourceSolution(Solution* solution) { source_solution_ = solution; }
    virtual void setTargetSolution(Solution* solution) { target_solution_ = solution; }
    
protected:
    Solution* source_solution_ = nullptr;
    Solution* target_solution_ = nullptr;
};

class DataExchangeInterface {
public:
    virtual ~DataExchangeInterface() = default;
    
    virtual bool canReceiveData(const std::string& data_type) const = 0;
    virtual bool canSendData(const std::string& data_type) const = 0;
    virtual void* receiveData(void* data, const std::string& data_type) = 0;
    virtual void* sendData(const std::string& data_type) = 0;
    
    virtual void registerDataExchange(DataExchange* exchange);
    virtual std::vector<DataExchange*> getDataExchanges() const { return data_exchanges_; }
    
private:
    std::vector<DataExchange*> data_exchanges_;
};

#endif // DATA_EXCHANGE_H

