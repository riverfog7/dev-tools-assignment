#include "ofApp.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>

namespace {
std::string coordToString(const Maze::Coord& coord) {
    std::ostringstream out;
    out << "(";
    for (std::size_t i = 0; i < coord.size(); ++i) {
        if (i > 0) {
            out << ", ";
        }
        out << coord[i];
    }
    out << ")";
    return out.str();
}
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    ofEnableAlphaBlending();
    setupNewGame();

}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(24);

    if (game_.hasMaze() && hasDrawableSlice()) {
        if (keepPlayerVisible_) {
            syncHiddenAxesToPlayer();
        }

        const std::vector<int> axes = selectedAxes();
        const MazeSlice2D slice = game_.maze().slice2D(axes[0], axes[1], sliceCoord_);

        const float renderWidth = std::max(80.0f, ofGetWidth() - guiWidth_ - margin_ * 3.0f);
        const float renderHeight = std::max(80.0f, ofGetHeight() - margin_ * 2.0f);
        renderer_.setBounds(margin_, margin_, renderWidth, renderHeight);

        renderer_.drawMaze(slice);
        if (showSolutionPath_) {
            renderer_.drawPath(slice, game_.solution(), ofColor(255, 205, 80, 110), 4.0f);
        }
        renderer_.drawPath(slice, game_.playerPath(), ofColor(65, 170, 255, 190), 5.0f);
        renderer_.drawMarker(slice, game_.start(), ofColor(75, 220, 130), 8.0f);
        renderer_.drawMarker(slice, game_.goal(), ofColor(240, 85, 85), 9.0f);
        renderer_.drawMarker(slice, game_.player(), ofColor(70, 150, 255), 11.0f);
    } else {
        ofDrawBitmapStringHighlight("Select exactly two axes to show a maze slice.", margin_, margin_);
    }

    drawGui();
    drawHud();

}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (!hasDrawableSlice()) {
        return;
    }

    const std::vector<int> axes = selectedAxes();
    bool moved = false;

    if (key == 'w' || key == 'W' || key == OF_KEY_UP) {
        moved = tryMove(axes[1], false);
    } else if (key == 's' || key == 'S' || key == OF_KEY_DOWN) {
        moved = tryMove(axes[1], true);
    } else if (key == 'a' || key == 'A' || key == OF_KEY_LEFT) {
        moved = tryMove(axes[0], false);
    } else if (key == 'd' || key == 'D' || key == OF_KEY_RIGHT) {
        moved = tryMove(axes[0], true);
    }

    if (moved) {
        if (keepPlayerVisible_) {
            syncHiddenAxesToPlayer();
        }
    }

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    if (draggedSliderAxis_ >= 0) {
        updateSliderFromMouse(draggedSliderAxis_, static_cast<float>(x));
    }

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    const glm::vec2 mouse(x, y);

    if (solutionToggleRect().inside(mouse)) {
        showSolutionPath_ = !showSolutionPath_;
        return;
    }

    if (playerVisibleToggleRect().inside(mouse)) {
        keepPlayerVisible_ = !keepPlayerVisible_;
        if (keepPlayerVisible_) {
            syncHiddenAxesToPlayer();
        }
        return;
    }

    if (saveButtonRect().inside(mouse)) {
        saveMaze();
        return;
    }

    if (loadButtonRect().inside(mouse)) {
        loadMaze();
        return;
    }

    if (!game_.hasMaze()) {
        return;
    }

    for (int axis = 0; axis < game_.maze().dim(); ++axis) {
        if (axisButtonRect(axis).inside(mouse)) {
            axisSelected_[axis] = !axisSelected_[axis];
            if (keepPlayerVisible_ && hasDrawableSlice()) {
                syncHiddenAxesToPlayer();
            }
            return;
        }

        if (sliderTrackRect(axis).inside(mouse)) {
            draggedSliderAxis_ = axis;
            updateSliderFromMouse(axis, static_cast<float>(x));
            return;
        }
    }

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    draggedSliderAxis_ = -1;

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::setupNewGame() {
    const std::vector<int> dimensions = {10, 10, 4, 3};
    game_.generate(dimensions, 123);
    prepareDefaultGame();
    resetAxisUi();
    statusText_ = "Ready";
}

void ofApp::prepareDefaultGame() {
    Maze::Coord start(game_.maze().dim(), 0);
    Maze::Coord goal;
    goal.reserve(game_.maze().shape().size());

    for (const int size : game_.maze().shape()) {
        goal.push_back(size - 1);
    }

    game_.prepareGame(start, goal);
    sliceCoord_ = game_.player();
}

void ofApp::resetAxisUi() {
    axisSelected_.assign(game_.maze().dim(), false);
    if (game_.maze().dim() > 0) {
        axisSelected_[0] = true;
    }
    if (game_.maze().dim() > 1) {
        axisSelected_[1] = true;
    }
}

std::vector<int> ofApp::selectedAxes() const {
    std::vector<int> axes;
    for (std::size_t axis = 0; axis < axisSelected_.size(); ++axis) {
        if (axisSelected_[axis]) {
            axes.push_back(static_cast<int>(axis));
        }
    }

    return axes;
}

bool ofApp::hasDrawableSlice() const {
    return game_.hasMaze() && selectedAxes().size() == 2;
}

bool ofApp::tryMove(const int axis, const bool positiveDirection) {
    return game_.move(axis, positiveDirection);
}

void ofApp::syncHiddenAxesToPlayer() {
    const std::vector<int> axes = selectedAxes();

    for (int axis = 0; axis < game_.maze().dim(); ++axis) {
        if (std::find(axes.begin(), axes.end(), axis) == axes.end()) {
            sliceCoord_[axis] = game_.player()[axis];
        }
    }
}

void ofApp::drawGui() const {
    const float panelX = guiX();
    const float panelHeight = ofGetHeight() - margin_ * 2.0f;

    ofSetColor(34);
    ofDrawRectangle(panelX, margin_, guiWidth_, panelHeight);

    ofSetColor(235);
    ofDrawBitmapString("Axes", panelX + guiPadding_, margin_ + 24.0f);

    if (game_.hasMaze()) {
        for (int axis = 0; axis < game_.maze().dim(); ++axis) {
            drawAxisRow(axis);
        }
    }

    drawButton(solutionToggleRect(), "Solution", showSolutionPath_);
    drawButton(playerVisibleToggleRect(), "Follow player", keepPlayerVisible_);
    drawButton(saveButtonRect(), "Save", false);
    drawButton(loadButtonRect(), "Load", false);
}

void ofApp::drawAxisRow(const int axis) const {
    const ofRectangle button = axisButtonRect(axis);
    const ofRectangle track = sliderTrackRect(axis);

    drawButton(button, std::to_string(axis), axisSelected_[axis]);

    const int maxValue = game_.maze().shape()[axis] - 1;
    const float t = maxValue > 0 ? static_cast<float>(sliceCoord_[axis]) / static_cast<float>(maxValue) : 0.0f;
    const float knobX = track.x + track.width * t;
    const float centerY = track.y + track.height * 0.5f;

    ofSetColor(88);
    ofDrawRectRounded(track.x, centerY - 3.0f, track.width, 6.0f, 3.0f);

    ofSetColor(axisSelected_[axis] ? ofColor(70, 150, 255) : ofColor(160));
    ofDrawRectRounded(track.x, centerY - 3.0f, knobX - track.x, 6.0f, 3.0f);
    ofDrawCircle(knobX, centerY, 8.0f);

    ofSetColor(220);
    ofDrawBitmapString(
        std::to_string(sliceCoord_[axis]) + " / " + std::to_string(maxValue),
        track.x,
        track.y + track.height + 15.0f
    );
}

void ofApp::drawButton(const ofRectangle& rect, const std::string& label, const bool active) const {
    ofSetColor(active ? ofColor(70, 150, 255) : ofColor(58));
    ofDrawRectRounded(rect, 6.0f);

    ofSetColor(245);
    const float textX = rect.x + 12.0f;
    const float textY = rect.y + rect.height * 0.5f + 5.0f;
    ofDrawBitmapString(label, textX, textY);
}

void ofApp::drawHud() const {
    if (!game_.hasMaze()) {
        return;
    }

    const float x = guiX() + guiPadding_;
    const float y = loadButtonRect().y + loadButtonRect().height + 28.0f;

    std::ostringstream text;
    text << "selected axes: ";
    const std::vector<int> axes = selectedAxes();
    for (std::size_t i = 0; i < axes.size(); ++i) {
        if (i > 0) {
            text << ", ";
        }
        text << axes[i];
    }
    text << "\n";
    text << "slice: " << coordToString(sliceCoord_) << "\n";
    text << "player: " << coordToString(game_.player()) << "\n";
    text << "goal: " << coordToString(game_.goal()) << "\n";
    text << "solved: " << (game_.solved() ? "yes" : "no") << "\n";
    text << statusText_;

    ofSetColor(235);
    ofDrawBitmapString(text.str(), x, y);
}

void ofApp::updateSliderFromMouse(const int axis, const float mouseX) {
    const ofRectangle track = sliderTrackRect(axis);
    const float t = ofClamp((mouseX - track.x) / track.width, 0.0f, 1.0f);
    const int maxValue = game_.maze().shape()[axis] - 1;
    sliceCoord_[axis] = static_cast<int>(std::round(t * maxValue));
}

void ofApp::saveMaze() {
    if (!game_.hasMaze()) {
        statusText_ = "No maze to save";
        return;
    }

    const ofFileDialogResult result = ofSystemSaveDialog("maze.ndmaze", "Save maze");
    if (!result.bSuccess) {
        return;
    }

    try {
        game_.exportToFile(result.filePath);
        statusText_ = "Saved " + result.fileName;
    } catch (const std::exception& error) {
        statusText_ = error.what();
    }
}

void ofApp::loadMaze() {
    const ofFileDialogResult result = ofSystemLoadDialog("Load maze");
    if (!result.bSuccess) {
        return;
    }

    try {
        game_.importFromFile(result.filePath);
        prepareDefaultGame();
        resetAxisUi();
        statusText_ = "Loaded " + result.fileName;
    } catch (const std::exception& error) {
        statusText_ = error.what();
    }
}

float ofApp::guiX() const {
    return ofGetWidth() - guiWidth_ - margin_;
}

ofRectangle ofApp::axisButtonRect(const int axis) const {
    const float x = guiX() + guiPadding_;
    const float y = margin_ + 44.0f + axis * axisRowHeight_;
    return {x, y, 44.0f, 30.0f};
}

ofRectangle ofApp::sliderTrackRect(const int axis) const {
    const ofRectangle button = axisButtonRect(axis);
    const float x = button.x + button.width + 14.0f;
    const float width = guiWidth_ - guiPadding_ * 2.0f - button.width - 14.0f;
    return {x, button.y + 2.0f, width, 18.0f};
}

ofRectangle ofApp::solutionToggleRect() const {
    const float y = margin_ + 58.0f + axisSelected_.size() * axisRowHeight_;
    const float x = guiX() + guiPadding_;
    return {x, y, guiWidth_ - guiPadding_ * 2.0f, 34.0f};
}

ofRectangle ofApp::playerVisibleToggleRect() const {
    const ofRectangle solution = solutionToggleRect();
    return {solution.x, solution.y + solution.height + 10.0f, solution.width, solution.height};
}

ofRectangle ofApp::saveButtonRect() const {
    const ofRectangle toggle = playerVisibleToggleRect();
    const float y = toggle.y + toggle.height + 16.0f;
    const float x = guiX() + guiPadding_;
    const float width = (guiWidth_ - guiPadding_ * 3.0f) * 0.5f;
    return {x, y, width, 34.0f};
}

ofRectangle ofApp::loadButtonRect() const {
    const ofRectangle save = saveButtonRect();
    return {save.x + save.width + guiPadding_, save.y, save.width, save.height};
}
