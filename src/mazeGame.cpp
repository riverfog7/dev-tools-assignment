//
// Created by 최재원 on 6/23/26.
//

#include "mazeGame.h"

#include <fstream>
#include <stdexcept>
#include <utility>

MazeGame::MazeGame() = default;

MazeGame::MazeGame(const std::vector<int>& dimensions) {
    generate(dimensions);
}

MazeGame::MazeGame(const std::vector<int>& dimensions, const unsigned int seed) {
    generate(dimensions, seed);
}

MazeGame::MazeGame(const std::string& path) {
    importFromFile(path);
}

void MazeGame::generate(const std::vector<int>& dimensions) {
    maze_ = Maze(dimensions);
    hasMaze_ = true;
    clearGameState();
}

void MazeGame::generate(const std::vector<int>& dimensions, const unsigned int seed) {
    maze_ = Maze(dimensions, seed);
    hasMaze_ = true;
    clearGameState();
}

void MazeGame::importFromFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Could not open maze file for reading");
    }

    Maze maze;
    maze.deserialize(in);

    maze_ = std::move(maze);
    hasMaze_ = true;
    clearGameState();
}

void MazeGame::exportToFile(const std::string& path) const {
    if (!hasMaze_) {
        throw std::runtime_error("No maze to export");
    }

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Could not open maze file for writing");
    }

    maze_.serialize(out);
}

void MazeGame::prepareGame(const Maze::Coord& start, const Maze::Coord& goal) {
    if (!hasMaze_) {
        throw std::runtime_error("No maze loaded");
    }

    const std::vector<Maze::Coord> solution = maze_.solve(start, goal);

    start_ = start;
    player_ = start;
    goal_ = goal;
    solution_ = solution;
    playerPath_.clear();
    playerPath_.push_back(player_);
    gamePrepared_ = true;
}

const Maze& MazeGame::maze() const {
    return maze_;
}

const Maze::Coord& MazeGame::start() const {
    return start_;
}

const Maze::Coord& MazeGame::player() const {
    return player_;
}

const Maze::Coord& MazeGame::goal() const {
    return goal_;
}

const std::vector<Maze::Coord>& MazeGame::solution() const {
    return solution_;
}

const std::vector<Maze::Coord>& MazeGame::playerPath() const {
    return playerPath_;
}

bool MazeGame::move(const int axis, const bool positiveDirection) {
    if (!canMove(axis, positiveDirection)) {
        return false;
    }

    Maze::Coord next = player_;
    next[axis] += positiveDirection ? 1 : -1;

    if (playerPath_.size() >= 2 && playerPath_[playerPath_.size() - 2] == next) {
        playerPath_.pop_back();
    } else {
        playerPath_.push_back(next);
    }

    player_ = next;
    return true;
}

bool MazeGame::canMove(const int axis, const bool positiveDirection) const {
    if (!gamePrepared_) {
        return false;
    }

    if (axis < 0 || axis >= maze_.dim()) {
        return false;
    }

    if (positiveDirection && player_[axis] + 1 >= maze_.shape()[axis]) {
        return false;
    }

    if (!positiveDirection && player_[axis] <= 0) {
        return false;
    }

    return !maze_(player_).hasWall(axis, positiveDirection);
}

bool MazeGame::solved() const {
    return gamePrepared_ && player_ == goal_;
}

bool MazeGame::hasMaze() const {
    return hasMaze_;
}

void MazeGame::clearGameState() {
    start_.clear();
    player_.clear();
    goal_.clear();
    solution_.clear();
    playerPath_.clear();
    gamePrepared_ = false;
}
