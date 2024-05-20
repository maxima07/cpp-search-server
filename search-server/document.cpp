#include "document.h"

Document::Document() = default;

Document::Document(int doc_id, double doc_relevance, int doc_rating)
        : id(doc_id)
        , relevance(doc_relevance)
        , rating(doc_rating) {
    }

std::ostream& operator<<(std::ostream& out, const Document& document){
    out << "{ " 
        << "document_id = " << document.id << ", "
        << "relevance = " << document.relevance << ", "
        << "rating = " << document.rating
        << " }";
    return out;
}