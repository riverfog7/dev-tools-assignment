//
// Created by riverfog7 on 6/22/2026.
//

#include "maze.h"

#include <stdexcept>

Maze::MazeCell::MazeCell(const std::uint64_t walls, const int dimensions)
    : walls_(walls), dimensions_(dimensions)
{
}

bool Maze::MazeCell::hasWall(const int axis, const bool positiveDirection) const {
    return (walls_ & wallBit(axis, positiveDirection)) != 0;
}

std::uint64_t Maze::MazeCell::rawWalls() const {
    return walls_;
}

std::uint64_t Maze::MazeCell::wallBit(const int axis, const bool positiveDirection) const {
    // returns bitmask for the specified wall
    if (axis < 0 || axis >= dimensions_) {
        throw std::out_of_range("MazeCell axis is out of bounds");
    }

    const int directionOffset = positiveDirection ? 0 : 1;
    return 1 << (axis * 2 + directionOffset);
}

Maze::Maze(std::vector<int> dimensions)
    : Maze(std::move(dimensions), std::random_device{}())
{
}

Maze::Maze(std::vector<int> dimensions, const unsigned int seed) : gen_(seed), dimensions_(std::move(dimensions)) {
}

const std::vector<int>& Maze::shape() const {
    return dimensions_;
}

int Maze::dim() const {
    // cpp style typecast
    return static_cast<int>(dimensions_.size());
}

Maze::MazeCell Maze::operator()(const Coord &coord) const {
    // returns MazeCell for the specified coord

    if (coord.size() != dimensions_.size()) {
        throw std::invalid_argument("Maze coordinate has wrong dimension count");
    }

    // always use size_t to hold indexes
    std::size_t index = 0;
    std::size_t stride = 1;
    for (std::size_t axis = 0; axis < dimensions_.size(); ++axis) {
        if (coord[axis] < 0 || coord[axis] >= dimensions_[axis]) {
            throw std::out_of_range("Maze coordinate is out of bounds");
        }

        index += static_cast<std::size_t>(coord[axis]) * stride;
        stride *= static_cast<std::size_t>(dimensions_[axis]);
    }

    // .at() is bounds checked
    return MazeCell(walls_.at(index), dim());
}
