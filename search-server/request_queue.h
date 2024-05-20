#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

#include "document.h"
#include "search_server.h"

class RequestQueue {
    public:
        explicit RequestQueue(const SearchServer& search_server);

        // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
        template <typename DocumentPredicate>   
        std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
            const auto result = search_server_.FindTopDocuments(raw_query, document_predicate);
            AddRequest(result.size());
            return result;
        }

        std::vector<Document> AddFindRequest(const  std::string& raw_query, DocumentStatus status);
        
        std::vector<Document> AddFindRequest(const  std::string& raw_query);

        // Возвращаем колличество пусты запросов
        int GetNoResultRequests() const;

    private:
        
        struct QueryResult {
            // Счетчик времени
            std::uint64_t timestamp;
            // Колличество запросов
            int result;
        };

        // Очередь запросов
        std::deque<QueryResult> requests_;
        
        // Максимальное кол-во запросов в сутки
        const static int min_in_day_ = 1440;
        
        // Ссылка на поисковый сервер
        const SearchServer& search_server_;

        // текущее времени
        uint64_t current_time_;

        // Счетчик запросов с пустым ответом
        int empty_results_;

        void AddRequest(int result_num);
};