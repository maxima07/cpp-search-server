#pragma once

#include <ostream>

struct Document {
    Document();
    
    Document(int doc_id, double doc_relevance, int doc_rating);

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

// Вывод документа на печать
std::ostream& operator<<(std::ostream& out, const Document& document);

