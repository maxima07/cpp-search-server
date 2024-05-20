#pragma once

#include <iostream>
#include <iterator>
#include <vector>

//Класс хранения итераторов одной страницы
template <typename Iterator>
class IteratorRange {
    public:
        //Неявный конструктор
        IteratorRange(){
        }

        //Явный конструктор
        IteratorRange(const Iterator& begin, const Iterator& end, const size_t size) 
            : begin_(begin)
            , end_(end)
            , size_(size){
        }

        //Итератор на начало контейнера
        Iterator begin() const {
            return begin_;
        }

        //Итератор на конец контейнера
        Iterator end() const {
            return end_;
        }

        //Размер контейнера
        std::size_t size() const {
            return size_;
        }

    private:
        Iterator begin_; 
        Iterator end_;
        std::size_t size_;
};

//Класс хранения вектора страниц
template <typename Iterator>
class Paginator {
    public:
        //неявный конструктор
        Paginator(){
        }

        //Явный конструктор
        explicit Paginator (Iterator it_begin, Iterator it_end, size_t page_size){
                
            std::size_t container_size = distance(it_begin, it_end);

            while (container_size >= page_size){
                Iterator page_begin = it_begin;
                std::advance (it_begin, page_size);
                Iterator page_end = it_begin;
                pages_.push_back({page_begin, page_end, page_size});
                container_size = distance(it_begin, it_end);
            }
            if (std::distance(it_begin, it_end) > 0) {
                std::size_t dist = distance(it_begin, it_end);
                pages_.push_back({it_begin, it_end, dist});
            }
        }

        auto begin() const {
            return pages_.begin();
        }

        auto end() const {
            return pages_.end();
        }

        std::size_t size() const {
            return pages_.size();
        }
    private:
        std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& range) {
    for(auto it = range.begin(); it != range.end(); ++it){
        out << *it;
    }
    return out; 
}

template <typename Container>
auto Paginate(const Container& c, std::size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}