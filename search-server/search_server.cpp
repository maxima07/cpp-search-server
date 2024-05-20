#include "search_server.h"

SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text)){
}

// Функция добавления документов
void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,
                const std::vector<int>& ratings) {
    if(document_id < 0 || count(document_ids_.begin(), document_ids_.end(), document_id)){
        throw std::invalid_argument("Document ID is wrong or a document with this ID has already been added earlier");
    } 

    const std::vector<std::string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
            
    for (const std::string& word : words) {
        if(!IsValidWord(word)){
            throw std::invalid_argument("The document has invalid characters");
        }
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
            
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.push_back(document_id);
}

// Переопределение функции поиска топа документов с заданным статусом документов
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;});
}

// Переопределение функции поиска топа документов 
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

// Получение колличества документов в базе
int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

// Получение ID доукента по его индексу
int SearchServer::GetDocumentId(int index) const {
    return document_ids_.at(index);
}

// Поиск на совпадуние запросу
std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query,
                                                        int document_id) const {
    Query query = ParseQuery(raw_query);
    std::vector<std::string> matched_words;
            
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return {matched_words, documents_.at(document_id).status};
}

// Проверка слова, является ли оно стоп-словом
bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

// Подсчет среднего рейтинга
int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

// Парсинг запроса
SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
    bool is_minus = false;
    
    if(!IsValidWord(text)){
        throw std::invalid_argument("The request has invalid characters");
    }

    if(!IsValidWord(text) || (text[0] == '-' && text.size() == 1) || (text[0] == '-' && text[1] == '-')){
        throw std::invalid_argument("The query has extra characters before or after the word");
    }

    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    return {text, is_minus, IsStopWord(text)};
}

// Парсинг запроса
SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
    Query query;
        
    for (const std::string& word : SplitIntoWords(text)) {
        QueryWord query_word = ParseQueryWord(word);

        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            } else {
                query.plus_words.insert(query_word.data);
            }
        }
    }
    return query;
}

// Проверка слова на валидность
bool SearchServer::IsValidWord(const std::string& word) {
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

// Подсчет IDF
double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}