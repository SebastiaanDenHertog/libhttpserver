#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include <string>
#include <stdexcept>
#include <iostream>

class StringView
{
public:
    // Constructors
    StringView() : data_(nullptr), size_(0) {}
    StringView(const char *str) : data_(str), size_(std::char_traits<char>::length(str)) {}
    StringView(const std::string &str) : data_(str.data()), size_(str.size()) {}
    StringView(const char *str, size_t len) : data_(str), size_(len) {}

    const char *data() const { return data_; }
    size_t size() const { return size_; }

    // Element access
    const char &operator[](size_t pos) const
    {
        return data_[pos];
    }

    const char &at(size_t pos) const
    {
        if (pos >= size_)
        {
            throw std::out_of_range("StringView: out of range");
        }
        return data_[pos];
    }

    bool empty() const
    {
        return size_ == 0;
    }

    // Substring method
    StringView substr(size_t pos, size_t len = std::string::npos) const
    {
        if (pos > size_)
        {
            throw std::out_of_range("StringView: out of range");
        }
        len = std::min(len, size_ - pos);
        return StringView(data_ + pos, len);
    }

    // Comparison operators
    bool operator==(const StringView &other) const
    {
        return size_ == other.size_ && std::equal(data_, data_ + size_, other.data_);
    }

    bool operator!=(const StringView &other) const
    {
        return !(*this == other);
    }

private:
    const char *data_;
    size_t size_;
};

#endif // STRING_VIEW_H