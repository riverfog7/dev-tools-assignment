//
// Created by riverfog7 on 6/22/2026.
//

#ifndef DEV_TOOLS_ASSIGNMENT_MAZE_H
#define DEV_TOOLS_ASSIGNMENT_MAZE_H

#include <iostream>
#include <random>
#include <cstdint>
#include <vector>

class Maze
{
public:
    class MazeCell {
    public:
        bool hasWall(int axis, bool positiveDirection) const;
        std::uint64_t rawWalls() const;

    private:
        // share private members with Maze class
        friend class Maze;

        MazeCell(std::uint64_t walls, int dimensions);

        std::uint64_t walls_;
        int dimensions_;

        std::uint64_t wallBit(int axis, bool positiveDirection) const;
    };
    using Coord = std::vector<int>;

    Maze(std::vector<int> dimensions);
    Maze(std::vector<int> dimensions, unsigned int seed);
    const std::vector<int>& shape() const;
    int dim() const;
    MazeCell operator()(const Coord &coord) const;

private:
    std::mt19937 gen_;
    std::vector<int> dimensions_;

    // holds wall info for each cell
    // a single integer represents wall state for a single cell
    // LSB: axis1 + wall exists
    // LSB+1: axis1 - wall exists
    // LSB+2: axis2 + wall exists...
    std::vector<std::uint64_t> walls_;

    std::ostream& serialize(std::ostream& out) const;
    std::istream& deserialize(std::istream& in);

    void generateRandomizedDfs();
};


#endif //DEV_TOOLS_ASSIGNMENT_MAZE_H
