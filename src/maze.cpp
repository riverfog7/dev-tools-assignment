//
// Created by riverfog7 on 6/22/2026.
//

#include "maze.h"

#include <limits>
#include <stack>
#include <stdexcept>
#include <utility>

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

std::size_t Maze::cellCount() const {
    std::size_t count = 1;
    for (const int size : dimensions_) {
        count *= static_cast<std::size_t>(size);
    }

    return count;
}

std::size_t Maze::indexOf(const Coord &coord) const {
    if (coord.size() != dimensions_.size()) {
        throw std::invalid_argument("Maze coordinate has wrong dimension count");
    }

    std::size_t index = 0;
    std::size_t stride = 1;
    for (std::size_t axis = 0; axis < dimensions_.size(); ++axis) {
        if (coord[axis] < 0 || coord[axis] >= dimensions_[axis]) {
            throw std::out_of_range("Maze coordinate is out of bounds");
        }

        index += static_cast<std::size_t>(coord[axis]) * stride;
        stride *= static_cast<std::size_t>(dimensions_[axis]);
    }

    return index;
}

Maze::Coord Maze::coordOf(const std::size_t index) const {
    if (index >= cellCount()) {
        throw std::out_of_range("Maze index is out of bounds");
    }

    Coord coord(dimensions_.size(), 0);
    std::size_t remaining = index;
    for (std::size_t axis = 0; axis < dimensions_.size(); ++axis) {
        coord[axis] = static_cast<int>(remaining % static_cast<std::size_t>(dimensions_[axis]));
        remaining /= static_cast<std::size_t>(dimensions_[axis]);
    }

    return coord;
}

std::uint64_t Maze::fullWallMask() const {
    const int bitCount = dim() * 2;
    if (bitCount == 64) {
        return std::numeric_limits<std::uint64_t>::max();
    }

    // 000...010...000 - 1
    // = 000...001...111
    return (std::uint64_t{1} << bitCount) - 1;
}

bool Maze::hasNeighbor(const std::size_t index, const int axis, const bool positiveDirection) const {
    if (axis < 0 || axis >= dim()) {
        throw std::out_of_range("Maze axis is out of bounds");
    }

    const Coord coord = coordOf(index);
    return positiveDirection ? coord[axis] + 1 < dimensions_[axis] : coord[axis] > 0;
}

std::size_t Maze::neighborIndex(const std::size_t index, const int axis, const bool positiveDirection) const {
    if (!hasNeighbor(index, axis, positiveDirection)) {
        throw std::out_of_range("Maze neighbor index is out of bounds");
    }

    Coord coord = coordOf(index);
    coord[axis] += positiveDirection ? 1 : -1;
    return indexOf(coord);
}

void Maze::removeWall(const std::size_t index, const int axis, const bool positiveDirection) {
    const MazeCell cell(walls_.at(index), dim());
    walls_.at(index) &= ~cell.wallBit(axis, positiveDirection);
}

void Maze::removeWallBetween(const std::size_t index, const int axis, const bool positiveDirection) {
    const std::size_t neighbor = neighborIndex(index, axis, positiveDirection);
    removeWall(index, axis, positiveDirection);
    removeWall(neighbor, axis, !positiveDirection);
}

std::vector<Maze::Neighbor> Maze::unvisitedNeighbors(const std::size_t index, const std::vector<bool> &visited) const {
    std::vector<Neighbor> neighbors;

    for (int axis = 0; axis < dim(); ++axis) {
        for (const bool positiveDirection : {true, false}) {
            if (!hasNeighbor(index, axis, positiveDirection)) {
                continue;
            }

            const std::size_t neighbor = neighborIndex(index, axis, positiveDirection);
            if (!visited.at(neighbor)) {
                neighbors.push_back({neighbor, axis, positiveDirection});
            }
        }
    }

    return neighbors;
}

std::optional<Maze::Neighbor> Maze::randomUnvisitedNeighbor(const std::size_t index, const std::vector<bool> &visited) {
    // optionally return a random unvisited neighbor, or std::nullopt if there are none
    const std::vector<Neighbor> neighbors = unvisitedNeighbors(index, visited);

    if (neighbors.empty()) {
        return std::nullopt;
    }

    std::uniform_int_distribution<std::size_t> distribution(0, neighbors.size() - 1);
    return neighbors[distribution(gen_)];
}

void Maze::generateRandomizedDfs() {
    walls_.assign(cellCount(), fullWallMask());

    std::vector visited(cellCount(), false);
    std::stack<std::size_t> stack;

    visited[0] = true;
    stack.push(0);

    while (!stack.empty()) {
        const std::size_t current = stack.top();
        const auto next = randomUnvisitedNeighbor(current, visited);

        if (!next) {
            stack.pop();
            continue;
        }

        removeWallBetween(current, next->axis, next->positiveDirection);
        visited[next->index] = true;
        stack.push(next->index);
    }
}
