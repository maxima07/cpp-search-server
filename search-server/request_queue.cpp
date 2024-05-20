#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server) 
            : search_server_(search_server)
            , current_time_(0)
            , empty_results_(0) {
}

std::vector<Document> RequestQueue::AddFindRequest(const  std::string& raw_query, DocumentStatus status) {
    const auto result = search_server_.FindTopDocuments(raw_query, status);
    RequestQueue::AddRequest(result.size());
    return result;
}

std::vector<Document> RequestQueue::AddFindRequest(const  std::string& raw_query) {
    const auto result = search_server_.FindTopDocuments(raw_query);
    RequestQueue::AddRequest(result.size());
    return result;
}

// Возвращаем колличество пусты запросов
int RequestQueue::GetNoResultRequests() const {
    return empty_results_;
}

void RequestQueue::AddRequest(int result_num){
    // Новый запрос -> увеличение текущего времени
    ++current_time_;
            
    // Удаляем результаты запросов, если дэк переполнен
    while(!requests_.empty() && min_in_day_ <= current_time_ - requests_.front().timestamp){
        // Если удаляемый результат пуст, корректируем счетчик запросов, удаляем запрос сначала дэка
        if(requests_.front().result == 0){
            --empty_results_;
        }
            requests_.pop_front();
    }

    // Добавляем поступивший запрос в конец
    requests_.push_back({current_time_, result_num});

    // Если новый запрос пустой, корректируем счетчик запросов
    if(result_num == 0){
        ++empty_results_;
    }
}