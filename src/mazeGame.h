//
// Created by 최재원 on 6/23/26.
//

#ifndef DEV_TOOLS_ASSIGNMENT_MAZEGAME_HPP
#define DEV_TOOLS_ASSIGNMENT_MAZEGAME_HPP

#include "maze.h"

#include <string>
#include <vector>

class MazeGame {
public:
    MazeGame();
    explicit MazeGame(const std::vector<int>& dimensions);
    MazeGame(const std::vector<int>& dimensions, unsigned int seed);
    explicit MazeGame(const std::string& path);

    void generate(const std::vector<int>& dimensions);
    void generate(const std::vector<int>& dimensions, unsigned int seed);

    void importFromFile(const std::string& path);
    void exportToFile(const std::string& path) const;

    void prepareGame(const Maze::Coord& start, const Maze::Coord& goal);

    const Maze& maze() const;
    const Maze::Coord& start() const;
    const Maze::Coord& player() const;
    const Maze::Coord& goal() const;
    const std::vector<Maze::Coord>& solution() const;
    const std::vector<Maze::Coord>& playerPath() const;

    bool move(int axis, bool positiveDirection);
    bool canMove(int axis, bool positiveDirection) const;
    bool solved() const;
    bool hasMaze() const;

private:
    void clearGameState();

    Maze maze_;
    Maze::Coord start_;
    Maze::Coord player_;
    Maze::Coord goal_;
    std::vector<Maze::Coord> solution_;
    std::vector<Maze::Coord> playerPath_;
    bool hasMaze_ = false;
    bool gamePrepared_ = false;
};


#endif //DEV_TOOLS_ASSIGNMENT_MAZEGAME_HPP
