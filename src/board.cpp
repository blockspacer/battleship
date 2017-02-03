#include <iostream>
#include "board.hpp"

bool Board::query(coords_t coords)
{
    mark_queried(coords);
    return get(coords);
}

void Board::mark_queried(coords_t coords)
{
    queried_.set(coords_to_index(coords));
}

bool Board::is_queried(coords_t coords)
{
    return queried_.test(coords_to_index(coords));
}

bool Board::get(coords_t coords)
{
    return ships_.test(coords_to_index(coords));
}

void Board::set(coords_t coords)
{
    ships_.set(coords_to_index(coords));
}

void Board::reset(coords_t coords)
{
    Board::ships_.reset(coords_to_index(coords));
}

void Board::flip(coords_t coords)
{
    ships_.flip(coords_to_index(coords));
}

void Board::commit()
{
}

void Board::debug()
{
    draw(std::cerr);
}

void Board::draw(std::ostream& os)
{
    for (size_t row = 0; row < size; ++row)
    {
        for (size_t col = 0; col < size; ++col)
        {
            os << ships_[coords_to_index({row, col})];
        }
        os << "\n";
    }
}

size_t Board::coords_to_index(coords_t coords)
{
    return size*coords.first + coords.second;
}

coords_t Board::index_to_coords(size_t n)
{
    return {n / size, n % size};
}
