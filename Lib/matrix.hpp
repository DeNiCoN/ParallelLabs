#pragma once
#include <vector>
#include <ranges>

template<typename T, typename Container = std::vector<T>>
class Matrix
{
public:

    Matrix(std::size_t m, std::size_t n)
        : m_rows(m), m_columns(n), m_data(m*n)
    {

    }

    Matrix(const Matrix& rhs)
        : m_rows(rhs.m()), m_columns(rhs.n()), m_data(rhs.m_data)
    {

    }

    void swap(Matrix& rhs) noexcept
    {
        std::swap(m_rows, rhs.m_rows);
        std::swap(m_columns, rhs.m_columns);
        std::swap(m_data, rhs.m_data);
    }

    Matrix& operator=(Matrix rhs) {
        rhs.swap(*this);
        return *this;
    }

    Matrix(Matrix&& rhs)
        : m_rows(rhs.m()), m_columns(rhs.n()), m_data(std::move(rhs.m_data))
    {

    }

    auto operator==(const Matrix& rhs) const
    {
        if (m_columns != rhs.m_columns || m_rows != rhs.m_rows)
            return false;

        return m_data == rhs.m_data;
    }

    auto operator[](std::size_t i)
    {
        return std::ranges::subrange(
            std::next(std::begin(m_data), i*m_rows),
            std::next(std::begin(m_data), i*m_rows + m_columns)
            );
    }

    auto operator[](std::size_t i) const
    {
        return std::ranges::subrange(
            std::next(std::begin(m_data), i*m_rows),
            std::next(std::begin(m_data), i*m_rows + m_columns)
            );
    }

    auto begin() {
        return m_data.begin();
    }

    auto begin() const {
        return m_data.begin();
    }

    auto end() {
        return m_data.end();
    }

    auto end() const {
        return m_data.end();
    }

    std::size_t m() const { return m_rows; }
    std::size_t n() const { return m_columns; }

    const Container& data() const { return m_data; }

private:
    Container m_data;
    std::size_t m_rows;
    std::size_t m_columns;
};
