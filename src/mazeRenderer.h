//
// Created by 최재원 on 6/23/26.
//

#ifndef DEV_TOOLS_ASSIGNMENT_MAZERENDERER_HPP
#define DEV_TOOLS_ASSIGNMENT_MAZERENDERER_HPP

#include "maze.h"
#include "ofMain.h"

#include <vector>

class MazeRenderer2D {
public:
    void setBounds(float x, float y, float width, float height);

    void drawMaze(const MazeSlice2D& slice) const;
    void drawPath(const MazeSlice2D& slice, const std::vector<Maze::Coord>& path, const ofColor& color, float thickness) const;
    void drawMarker(const MazeSlice2D& slice, const Maze::Coord& coord, const ofColor& color, float radius) const;

private:
    float cellSize(const MazeSlice2D& slice) const;
    glm::vec2 cellTopLeft(const MazeSlice2D& slice, int x, int y) const;
    glm::vec2 cellCenter(const MazeSlice2D& slice, int x, int y) const;

    float x_ = 40.0f;
    float y_ = 40.0f;
    float width_ = 700.0f;
    float height_ = 700.0f;
};


#endif //DEV_TOOLS_ASSIGNMENT_MAZERENDERER_HPP
