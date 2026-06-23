//
// Created by 최재원 on 6/23/26.
//

#include "mazeRenderer.h"

#include <algorithm>

void MazeRenderer2D::setBounds(const float x, const float y, const float width, const float height) {
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

void MazeRenderer2D::drawMaze(const MazeSlice2D& slice) const {
    const float size = cellSize(slice);

    ofSetColor(235);
    ofSetLineWidth(2.0f);

    for (int y = 0; y < slice.height(); ++y) {
        for (int x = 0; x < slice.width(); ++x) {
            const Maze::MazeCell cell = slice.cellAt(x, y);
            const glm::vec2 topLeft = cellTopLeft(slice, x, y);
            const float left = topLeft.x;
            const float top = topLeft.y;
            const float right = left + size;
            const float bottom = top + size;

            if (cell.hasWall(slice.xAxis(), false)) {
                ofDrawLine(left, top, left, bottom);
            }
            if (cell.hasWall(slice.xAxis(), true)) {
                ofDrawLine(right, top, right, bottom);
            }
            if (cell.hasWall(slice.yAxis(), false)) {
                ofDrawLine(left, top, right, top);
            }
            if (cell.hasWall(slice.yAxis(), true)) {
                ofDrawLine(left, bottom, right, bottom);
            }
        }
    }
}

void MazeRenderer2D::drawPath(const MazeSlice2D& slice, const std::vector<Maze::Coord>& path, const ofColor& color, const float thickness) const {
    if (path.size() < 2) {
        return;
    }

    ofSetColor(color);
    ofSetLineWidth(thickness);

    for (std::size_t i = 1; i < path.size(); ++i) {
        const Maze::Coord& a = path[i - 1];
        const Maze::Coord& b = path[i];

        if (!slice.contains(a) || !slice.contains(b)) {
            continue;
        }

        const glm::vec2 aCenter = cellCenter(slice, a[slice.xAxis()], a[slice.yAxis()]);
        const glm::vec2 bCenter = cellCenter(slice, b[slice.xAxis()], b[slice.yAxis()]);
        ofDrawLine(aCenter.x, aCenter.y, bCenter.x, bCenter.y);
    }
}

void MazeRenderer2D::drawMarker(const MazeSlice2D& slice, const Maze::Coord& coord, const ofColor& color, const float radius) const {
    if (!slice.contains(coord)) {
        return;
    }

    const glm::vec2 center = cellCenter(slice, coord[slice.xAxis()], coord[slice.yAxis()]);
    ofSetColor(color);
    ofDrawCircle(center.x, center.y, radius);
}

float MazeRenderer2D::cellSize(const MazeSlice2D& slice) const {
    return std::min(width_ / slice.width(), height_ / slice.height());
}

glm::vec2 MazeRenderer2D::cellTopLeft(const MazeSlice2D& slice, const int x, const int y) const {
    const float size = cellSize(slice);
    return {x_ + x * size, y_ + y * size};
}

glm::vec2 MazeRenderer2D::cellCenter(const MazeSlice2D& slice, const int x, const int y) const {
    const glm::vec2 topLeft = cellTopLeft(slice, x, y);
    const float size = cellSize(slice);
    return {topLeft.x + size * 0.5f, topLeft.y + size * 0.5f};
}
