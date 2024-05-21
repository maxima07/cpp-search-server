#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "document.h"
#include "string_processing.h"


class SearchServer {
    
    public:   

        // Объявление конструктора класса SearchServer
        explicit SearchServer(const std::string& stop_words_text);

        // Объявление шаблонного конструктора класса SearchServer
        template <typename StringContainer>
        explicit SearchServer(const StringContainer& stop_words);

        // Функция добавления документов
        void AddDocument(int document_id, const std::string& document, DocumentStatus status,
            const std::vector<int>& ratings);

        // Объявление аблонной функции поиска топа документов с функцией предикатом
        template <typename DocumentPredicate>
        std::vector<Document> FindTopDocuments(const std::string& raw_query,
            DocumentPredicate document_predicate) const;

        // Переопределение функции поиска топа документов с заданным статусом документов
        std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;

        // Переопределение функции поиска топа документов 
        std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

        // Получение колличества документов в базе
        int GetDocumentCount() const;

        // Получение ID доукента по его индексу
        int GetDocumentId(int index) const;
        
        // Поиск на совпадуние запросу
        std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query,
            int document_id) const;

    private:

        struct DocumentData {
            int rating;
            DocumentStatus status;
        };

        const std::set<std::string> stop_words_;
        
        std::map<std::string, std::map<int, double>> word_to_document_freqs_;
        
        std::map<int, DocumentData> documents_;

        std::vector<int> document_ids_;

        // Проверка слова, является ли оно стоп-словом
        bool IsStopWord(const std::string& word) const;

        // Удаляем из запроса стоп-слова
        std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

        // Подсчет среднего рейтинга
        static int ComputeAverageRating(const std::vector<int>& ratings);

        struct QueryWord {
            std::string data;
            bool is_minus;
            bool is_stop;
        };
        
        // Парсинг запроса
        QueryWord ParseQueryWord(std::string text) const;

        struct Query {
            std::set<std::string> plus_words;
            std::set<std::string> minus_words;
        };

        // Парсинг запроса
        Query ParseQuery(const std::string& text) const;

        // Проверка слова на валидность
        static bool IsValidWord(const std::string& word);

        // Подсчет IDF
        double ComputeWordInverseDocumentFreq(const std::string& word) const;

        // Объявление Шаблонной функции поисхха всех документов соответствующих запросу
        template <typename DocumentPredicate>
        std::vector<Document> FindAllDocuments(const Query& query,
            DocumentPredicate& document_predicate) const;
};

// Реализация шаблонных функций

// Реализация шаблонного конструктора класса SearchServer
template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {

    if(std::any_of(stop_words.begin(), stop_words.end(), [](const std::string& word){return !IsValidWord(word);})){
        throw std::invalid_argument("Stop words have special symbols!");
    }      
}

// Реализация шаблонной функции поиска топа документов с функцией предикатом
template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query,
    DocumentPredicate document_predicate) const {
            
    const int MAX_RESULT_DOCUMENT_COUNT = 5;
    Query query = ParseQuery(raw_query);
    std::vector<Document> matched_documents = FindAllDocuments(query, document_predicate);

    std::sort(matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs) {
            const double EPSILON = 1e-6;

            if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
                return lhs.rating > rhs.rating;
            } else {
                return lhs.relevance > rhs.relevance;
            }
    });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

// Реализация аблонной функции поисхха всех документов соответствующих запросу
template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query,
    DocumentPredicate& document_predicate) const {

    std::map<int, double> document_to_relevance;
            
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
                
        for (const auto &[document_id, term_freq] : word_to_document_freqs_.at(word)) {
            const auto& document_data = documents_.at(document_id);
            
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }
    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }

        for (const auto &[document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }
    std::vector<Document> matched_documents;
    
    for (const auto &[document_id, relevance] : document_to_relevance) {
        matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
    }

    return matched_documents;
}