//
// Created by riverfog7 on 6/22/2026.
//

#include "maze.h"

#include <CommonCrypto/CommonDigest.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstring>
#include <iterator>
#include <limits>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <utility>

constexpr char MAZE_MAGIC[8] = {'N', 'D', 'M', 'A', 'Z', 'E', '\0', '\0'};
constexpr std::size_t MAZE_MAGIC_SIZE = 8;
constexpr std::size_t MD5_SIZE = 16;
constexpr int MAX_DIMENSIONS = 32;

// cast int values to bytes for writing to stream
// using bit cast (c++ 20)
void writeUint32(std::ostream& out, const std::uint32_t value) {
    const auto bytes = std::bit_cast<std::array<char, sizeof(value)>>(value);
    out.write(bytes.data(), bytes.size());
}

void writeInt32(std::ostream& out, const std::int32_t value) {
    const auto bytes = std::bit_cast<std::array<char, sizeof(value)>>(value);
    out.write(bytes.data(), bytes.size());
}

void writeUint64(std::ostream& out, const std::uint64_t value) {
    const auto bytes = std::bit_cast<std::array<char, sizeof(value)>>(value);
    out.write(bytes.data(), bytes.size());
}

std::uint32_t readUint32(std::istream& in) {
    std::array<char, sizeof(std::uint32_t)> bytes{};
    in.read(bytes.data(), bytes.size());
    if (!in) {
        throw std::runtime_error("Maze file is truncated");
    }
    return std::bit_cast<std::uint32_t>(bytes);
}

std::int32_t readInt32(std::istream& in) {
    std::array<char, sizeof(std::int32_t)> bytes{};
    in.read(bytes.data(), bytes.size());
    if (!in) {
        throw std::runtime_error("Maze file is truncated");
    }
    return std::bit_cast<std::int32_t>(bytes);
}

std::uint64_t readUint64(std::istream& in) {
    std::array<char, sizeof(std::uint64_t)> bytes{};
    in.read(bytes.data(), bytes.size());
    if (!in) {
        throw std::runtime_error("Maze file is truncated");
    }
    return std::bit_cast<std::uint64_t>(bytes);
}

std::size_t cellCountFor(const std::vector<int>& dimensions) {
    std::size_t count = 1;
    for (const int size : dimensions) {
        count *= static_cast<std::size_t>(size);
    }
    return count;
}

std::uint64_t fullWallMaskFor(const std::size_t dimensionCount) {
    const std::size_t bitCount = dimensionCount * 2;
    if (bitCount == MAX_DIMENSIONS * 2) {
        return std::numeric_limits<std::uint64_t>::max();
    }

    return (std::uint64_t{1} << bitCount) - 1;
}

std::array<unsigned char, MD5_SIZE> md5Digest(const std::string& bytes) {
    if (bytes.size() > std::numeric_limits<CC_LONG>::max()) {
        throw std::runtime_error("Maze data is too large to hash with MD5");
    }

    std::array<unsigned char, MD5_SIZE> digest{};
    CC_MD5(bytes.data(), static_cast<CC_LONG>(bytes.size()), digest.data());
    return digest;
}

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
    return std::uint64_t{1} << (axis * 2 + directionOffset);
}

Maze::Maze()
    : gen_(std::random_device{}())
{
}

Maze::Maze(std::vector<int> dimensions)
    : Maze(std::move(dimensions), std::random_device{}())
{
}

Maze::Maze(std::vector<int> dimensions, const unsigned int seed) : gen_(seed), dimensions_(std::move(dimensions)) {
    validateDimensions();
    generateRandomizedDfs();
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

std::ostream& Maze::serialize(std::ostream &out) const {
    if (walls_.size() != cellCount()) {
        throw std::runtime_error("Maze wall data does not match dimensions");
    }

    std::ostringstream payloadStream(std::ios::out | std::ios::binary);
    payloadStream.write(MAZE_MAGIC, MAZE_MAGIC_SIZE);

    const auto dimensionCount = static_cast<std::uint32_t>(dimensions_.size());
    writeUint32(payloadStream, dimensionCount);

    for (const int dimension : dimensions_) {
        writeInt32(payloadStream, dimension);
    }

    for (const std::uint64_t walls : walls_) {
        writeUint64(payloadStream, walls);
    }

    const std::string payload = payloadStream.str();
    const auto digest = md5Digest(payload);
    const auto digestBytes = std::bit_cast<std::array<char, MD5_SIZE>>(digest);

    out.write(payload.data(), static_cast<std::streamsize>(payload.size()));
    out.write(digestBytes.data(), digestBytes.size());

    return out;
}

std::istream& Maze::deserialize(std::istream &in) {
    const std::string data{
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>()
    };

    if (data.size() < MAZE_MAGIC_SIZE + MD5_SIZE) {
        throw std::runtime_error("Maze file is too small");
    }

    if (std::memcmp(data.data(), MAZE_MAGIC, MAZE_MAGIC_SIZE) != 0) {
        throw std::runtime_error("Maze file has invalid magic bytes");
    }

    const std::size_t payloadSize = data.size() - MD5_SIZE;
    const std::string payload = data.substr(0, payloadSize);
    const auto expectedDigest = md5Digest(payload);

    std::array<char, MD5_SIZE> actualDigestBytes{};
    for (std::size_t i = 0; i < MD5_SIZE; ++i) {
        actualDigestBytes[i] = data[payloadSize + i];
    }

    const auto actualDigest = std::bit_cast<std::array<unsigned char, MD5_SIZE>>(actualDigestBytes);
    if (expectedDigest != actualDigest) {
        throw std::runtime_error("Maze file checksum does not match");
    }

    std::istringstream payloadStream(payload, std::ios::in | std::ios::binary);
    payloadStream.seekg(MAZE_MAGIC_SIZE);

    const std::uint32_t dimensionCount = readUint32(payloadStream);

    if (dimensionCount == 0 || dimensionCount > MAX_DIMENSIONS) {
        throw std::runtime_error("Maze file has invalid dimension count");
    }

    std::vector<int> dimensions;
    dimensions.reserve(dimensionCount);
    for (std::uint32_t i = 0; i < dimensionCount; ++i) {
        const std::int32_t dimension = readInt32(payloadStream);
        if (dimension <= 0) {
            throw std::runtime_error("Maze file has invalid dimension size");
        }

        dimensions.push_back(static_cast<int>(dimension));
    }

    const std::size_t expectedWallCount = cellCountFor(dimensions);
    const std::uint64_t validWallMask = fullWallMaskFor(dimensions.size());

    std::vector<std::uint64_t> walls;
    walls.reserve(expectedWallCount);
    for (std::size_t i = 0; i < expectedWallCount; ++i) {
        const std::uint64_t wallBits = readUint64(payloadStream);
        if ((wallBits & ~validWallMask) != 0) {
            throw std::runtime_error("Maze file has invalid wall bits");
        }

        walls.push_back(wallBits);
    }

    if (payloadStream.tellg() != static_cast<std::streampos>(payload.size())) {
        throw std::runtime_error("Maze file has trailing payload bytes");
    }

    dimensions_ = std::move(dimensions);
    walls_ = std::move(walls);

    return in;
}

std::vector<Maze::Coord> Maze::solve(const Coord &start, const Coord &goal) const {
    const std::size_t startIndex = indexOf(start);
    const std::size_t goalIndex = indexOf(goal);

    std::vector visited(cellCount(), false);
    std::vector previous(cellCount(), cellCount());
    std::stack<std::size_t> stack;

    visited[startIndex] = true;
    stack.push(startIndex);

    while (!stack.empty()) {
        const std::size_t current = stack.top();
        stack.pop();

        if (current == goalIndex) {
            break;
        }

        const std::vector<Neighbor> neighbors = connectedUnvisitedNeighbors(current, visited);
        for (const Neighbor& neighbor : neighbors) {
            visited[neighbor.index] = true;
            previous[neighbor.index] = current;
            stack.push(neighbor.index);
        }
    }

    if (!visited[goalIndex]) {
        return {};
    }

    std::vector<Coord> path;
    for (std::size_t index = goalIndex; index != cellCount(); index = previous[index]) {
        path.push_back(coordOf(index));
        if (index == startIndex) {
            break;
        }
    }

    std::reverse(path.begin(), path.end());
    return path;
}

MazeSlice2D Maze::slice2D(const int xAxis, const int yAxis, const Coord &baseCoord) const {
    if (xAxis < 0 || xAxis >= dim() || yAxis < 0 || yAxis >= dim()) {
        throw std::out_of_range("Maze slice axis is out of bounds");
    }
    if (xAxis == yAxis) {
        throw std::invalid_argument("Maze slice axes must be different");
    }
    if (baseCoord.size() != dimensions_.size()) {
        throw std::invalid_argument("Maze slice base coordinate has wrong dimension count");
    }

    for (std::size_t axis = 0; axis < dimensions_.size(); ++axis) {
        if (static_cast<int>(axis) == xAxis || static_cast<int>(axis) == yAxis) {
            continue;
        }
        if (baseCoord[axis] < 0 || baseCoord[axis] >= dimensions_[axis]) {
            throw std::out_of_range("Maze slice base coordinate is out of bounds");
        }
    }

    return MazeSlice2D(this, xAxis, yAxis, baseCoord);
}

MazeSlice3D Maze::slice3D(const int xAxis, const int yAxis, const int zAxis, const Coord &baseCoord) const {
    if (xAxis < 0 || xAxis >= dim() || yAxis < 0 || yAxis >= dim() || zAxis < 0 || zAxis >= dim()) {
        throw std::out_of_range("Maze slice axis is out of bounds");
    }
    if (xAxis == yAxis || xAxis == zAxis || yAxis == zAxis) {
        throw std::invalid_argument("Maze slice axes must be different");
    }
    if (baseCoord.size() != dimensions_.size()) {
        throw std::invalid_argument("Maze slice base coordinate has wrong dimension count");
    }

    for (std::size_t axis = 0; axis < dimensions_.size(); ++axis) {
        if (static_cast<int>(axis) == xAxis || static_cast<int>(axis) == yAxis || static_cast<int>(axis) == zAxis) {
            continue;
        }
        if (baseCoord[axis] < 0 || baseCoord[axis] >= dimensions_[axis]) {
            throw std::out_of_range("Maze slice base coordinate is out of bounds");
        }
    }

    return MazeSlice3D(this, xAxis, yAxis, zAxis, baseCoord);
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

void Maze::validateDimensions() const {
    if (dimensions_.empty()) {
        throw std::invalid_argument("Maze must have at least one dimension");
    }
    if (dimensions_.size() > MAX_DIMENSIONS) {
        throw std::invalid_argument("Maze supports at most 32 dimensions");
    }
    for (const int size : dimensions_) {
        if (size <= 0) {
            throw std::invalid_argument("Maze dimensions must be positive");
        }
    }
}

std::uint64_t Maze::fullWallMask() const {
    const int bitCount = dim() * 2;
    if (bitCount == MAX_DIMENSIONS * 2) {
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

std::vector<Maze::Neighbor> Maze::connectedUnvisitedNeighbors(const std::size_t index, const std::vector<bool> &visited) const {
    std::vector<Neighbor> neighbors;
    const MazeCell cell(walls_.at(index), dim());

    for (int axis = 0; axis < dim(); ++axis) {
        for (const bool positiveDirection : {true, false}) {
            if (!hasNeighbor(index, axis, positiveDirection)) {
                continue;
            }

            if (cell.hasWall(axis, positiveDirection)) {
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

MazeSlice2D::MazeSlice2D(const Maze* maze, const int xAxis, const int yAxis, Maze::Coord baseCoord)
    : maze_(maze), xAxis_(xAxis), yAxis_(yAxis), baseCoord_(std::move(baseCoord))
{
}

int MazeSlice2D::xAxis() const {
    return xAxis_;
}

int MazeSlice2D::yAxis() const {
    return yAxis_;
}

int MazeSlice2D::width() const {
    return maze_->shape()[xAxis_];
}

int MazeSlice2D::height() const {
    return maze_->shape()[yAxis_];
}

Maze::Coord MazeSlice2D::coordAt(const int x, const int y) const {
    if (x < 0 || x >= width() || y < 0 || y >= height()) {
        throw std::out_of_range("Maze slice coordinate is out of bounds");
    }

    Maze::Coord coord = baseCoord_;
    coord[xAxis_] = x;
    coord[yAxis_] = y;
    return coord;
}

Maze::MazeCell MazeSlice2D::cellAt(const int x, const int y) const {
    return (*maze_)(coordAt(x, y));
}

bool MazeSlice2D::contains(const Maze::Coord& coord) const {
    if (coord.size() != maze_->shape().size()) {
        return false;
    }

    for (std::size_t axis = 0; axis < coord.size(); ++axis) {
        if (coord[axis] < 0 || coord[axis] >= maze_->shape()[axis]) {
            return false;
        }
        if (static_cast<int>(axis) != xAxis_ && static_cast<int>(axis) != yAxis_ && coord[axis] != baseCoord_[axis]) {
            return false;
        }
    }

    return true;
}

MazeSlice3D::MazeSlice3D(const Maze* maze, const int xAxis, const int yAxis, const int zAxis, Maze::Coord baseCoord)
    : maze_(maze), xAxis_(xAxis), yAxis_(yAxis), zAxis_(zAxis), baseCoord_(std::move(baseCoord))
{
}

int MazeSlice3D::xAxis() const {
    return xAxis_;
}

int MazeSlice3D::yAxis() const {
    return yAxis_;
}

int MazeSlice3D::zAxis() const {
    return zAxis_;
}

int MazeSlice3D::width() const {
    return maze_->shape()[xAxis_];
}

int MazeSlice3D::height() const {
    return maze_->shape()[yAxis_];
}

int MazeSlice3D::depth() const {
    return maze_->shape()[zAxis_];
}

Maze::Coord MazeSlice3D::coordAt(const int x, const int y, const int z) const {
    if (x < 0 || x >= width() || y < 0 || y >= height() || z < 0 || z >= depth()) {
        throw std::out_of_range("Maze slice coordinate is out of bounds");
    }

    Maze::Coord coord = baseCoord_;
    coord[xAxis_] = x;
    coord[yAxis_] = y;
    coord[zAxis_] = z;
    return coord;
}

Maze::MazeCell MazeSlice3D::cellAt(const int x, const int y, const int z) const {
    return (*maze_)(coordAt(x, y, z));
}

bool MazeSlice3D::contains(const Maze::Coord& coord) const {
    if (coord.size() != maze_->shape().size()) {
        return false;
    }

    for (std::size_t axis = 0; axis < coord.size(); ++axis) {
        if (coord[axis] < 0 || coord[axis] >= maze_->shape()[axis]) {
            return false;
        }
        if (static_cast<int>(axis) != xAxis_ && static_cast<int>(axis) != yAxis_ && static_cast<int>(axis) != zAxis_ && coord[axis] != baseCoord_[axis]) {
            return false;
        }
    }

    return true;
}
