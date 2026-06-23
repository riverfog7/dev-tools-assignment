#include "ofApp.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace {
constexpr int MIN_CONFIG_DIMENSIONS = 2;
constexpr int MAX_CONFIG_DIMENSIONS = 32;
constexpr int MIN_CONFIG_CELLS = 2;
constexpr int MAX_CONFIG_CELLS = 40;
constexpr unsigned int MAX_CONFIG_SEED = 999999;
constexpr std::size_t MAX_CONFIGURED_CELLS = 4000000;
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    ofEnableAlphaBlending();
    loadFonts();
    syncConfigDimensions();

}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(24);

    if (screen_ == Screen::Start) {
        drawStartScreen();
    } else if (screen_ == Screen::Configure) {
        drawConfigScreen();
    } else if (screen_ == Screen::Finished) {
        drawFinishScreen();
    } else {
        drawGameScreen();
    }

}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (screen_ != Screen::Playing) {
        return;
    }

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
        if (game_.solved()) {
            screen_ = Screen::Finished;
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
    if (screen_ != Screen::Playing) {
        return;
    }

    if (draggedSliderAxis_ >= 0) {
        updateSliderFromMouse(draggedSliderAxis_, static_cast<float>(x));
    }

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    const glm::vec2 mouse(x, y);

    if (screen_ == Screen::Start) {
        if (screenButtonRect(0).inside(mouse)) {
            setupNewGame();
            return;
        }
        if (screenButtonRect(1).inside(mouse)) {
            screen_ = Screen::Configure;
            return;
        }
        return;
    }

    if (screen_ == Screen::Configure) {
        if (configMinusButtonRect(0).inside(mouse) && configSeed_ > 0) {
            --configSeed_;
            configStatus_.clear();
            return;
        }
        if (configPlusButtonRect(0).inside(mouse) && configSeed_ < MAX_CONFIG_SEED) {
            ++configSeed_;
            configStatus_.clear();
            return;
        }
        if (configMinusButtonRect(1).inside(mouse) && configDimensionCount_ > MIN_CONFIG_DIMENSIONS) {
            --configDimensionCount_;
            syncConfigDimensions();
            clampConfigAxisScrollOffset();
            configStatus_.clear();
            return;
        }
        if (configPlusButtonRect(1).inside(mouse) && configDimensionCount_ < MAX_CONFIG_DIMENSIONS) {
            ++configDimensionCount_;
            syncConfigDimensions();
            clampConfigAxisScrollOffset();
            configStatus_.clear();
            return;
        }
        if (configAxisAreaRect().inside(mouse)) {
            for (int axis = 0; axis < configDimensionCount_; ++axis) {
                if (configAxisMinusButtonRect(axis).inside(mouse) && configDimensions_[axis] > MIN_CONFIG_CELLS) {
                    --configDimensions_[axis];
                    configStatus_.clear();
                    return;
                }
                if (configAxisPlusButtonRect(axis).inside(mouse) && configDimensions_[axis] < MAX_CONFIG_CELLS) {
                    ++configDimensions_[axis];
                    configStatus_.clear();
                    return;
                }
            }
        }
        if (screenButtonRect(4).inside(mouse)) {
            if (canGenerateConfig()) {
                setupNewGame();
            } else {
                configStatus_ = "Too many cells";
            }
            return;
        }
        if (screenButtonRect(5).inside(mouse)) {
            screen_ = Screen::Start;
            return;
        }
        return;
    }

    if (screen_ == Screen::Finished) {
        if (screenButtonRect(0).inside(mouse)) {
            prepareDefaultGame();
            resetAxisUi();
            statusText_ = "Ready";
            screen_ = Screen::Playing;
            return;
        }
        if (screenButtonRect(1).inside(mouse)) {
            screen_ = Screen::Configure;
            return;
        }
        return;
    }

    if (screen_ != Screen::Playing) {
        return;
    }

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

    if (axesAreaRect().inside(mouse)) {
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

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    draggedSliderAxis_ = -1;

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){
    if (screen_ == Screen::Configure) {
        if (!configAxisAreaRect().inside(glm::vec2(x, y))) {
            return;
        }

        const float nextScrollOffset = ofClamp(
            configAxisScrollOffset_ - scrollY * configAxisRowHeight_,
            0.0f,
            maxConfigAxisScrollOffset()
        );
        configAxisScrollOffset_ = ofClamp(
            std::round(nextScrollOffset / configAxisRowHeight_) * configAxisRowHeight_,
            0.0f,
            maxConfigAxisScrollOffset()
        );
        return;
    }

    if (screen_ != Screen::Playing) {
        return;
    }

    if (axesAreaRect().inside(glm::vec2(x, y))) {
        axisScrollOffset_ = ofClamp(
            axisScrollOffset_ - scrollY * axisRowHeight_,
            0.0f,
            maxAxisScrollOffset()
        );
        return;
    }

    if (hudAreaRect().inside(glm::vec2(x, y))) {
        hudScrollOffset_ = ofClamp(
            hudScrollOffset_ - scrollY * hudLineHeight_ * 3.0f,
            0.0f,
            maxHudScrollOffset()
        );
    }

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    clampAxisScrollOffset();
    clampConfigAxisScrollOffset();
    clampHudScrollOffset();

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::setupNewGame() {
    if (!canGenerateConfig()) {
        configStatus_ = "Too many cells";
        screen_ = Screen::Configure;
        return;
    }

    try {
        game_.generate(configDimensions_, configSeed_);
        prepareDefaultGame();
        resetAxisUi();
        showSolutionPath_ = false;
        keepPlayerVisible_ = true;
        configStatus_.clear();
        statusText_ = "Ready";
        screen_ = Screen::Playing;
    } catch (const std::exception& error) {
        configStatus_ = error.what();
        screen_ = Screen::Configure;
    }
}

void ofApp::syncConfigDimensions() {
    configDimensionCount_ = ofClamp(configDimensionCount_, MIN_CONFIG_DIMENSIONS, MAX_CONFIG_DIMENSIONS);

    if (configDimensions_.empty()) {
        configDimensions_.assign(configDimensionCount_, 10);
        return;
    }

    const int fillValue = configDimensions_.back();
    configDimensions_.resize(configDimensionCount_, fillValue);

    for (int& size : configDimensions_) {
        size = ofClamp(size, MIN_CONFIG_CELLS, MAX_CONFIG_CELLS);
    }
}

void ofApp::loadFonts() {
    titleFont_.load("fonts/Pretendard-SemiBold.otf", 32, true, true);
    bodyFont_.load("fonts/Pretendard-Regular.otf", 15, true, true);
    smallFont_.load("fonts/Pretendard-Regular.otf", 12, true, true);
    buttonFont_.load("fonts/Pretendard-SemiBold.otf", 14, true, true);
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
    axisScrollOffset_ = 0.0f;
    hudScrollOffset_ = 0.0f;

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

void ofApp::drawGameScreen() {
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
        ofSetColor(235);
        drawText("Select exactly two axes to show a maze slice.", margin_, margin_ + 16.0f, bodyFont_);
    }

    drawGui();
    drawHud();
}

void ofApp::drawStartScreen() const {
    ofSetColor(240);
    drawCenteredText("N-D Maze", {0.0f, ofGetHeight() * 0.5f - 162.0f, static_cast<float>(ofGetWidth()), 60.0f}, titleFont_);

    drawButton(screenButtonRect(0), "Start", true);
    drawButton(screenButtonRect(1), "Configure", false);
}

void ofApp::drawConfigScreen() const {
    const ofRectangle content = screenButtonRect(0);
    const ofRectangle axisArea = configAxisAreaRect();
    const float panelX = content.x - 48.0f;
    const float panelY = 52.0f;
    const float panelWidth = content.width + 96.0f;
    const float panelHeight = ofGetHeight() - panelY * 2.0f;

    ofSetColor(34);
    ofDrawRectRounded(panelX, panelY, panelWidth, panelHeight, 8.0f);

    ofSetColor(240);
    drawText("Configure Maze", content.x, panelY + 66.0f, titleFont_);

    drawConfigRow("Seed", std::to_string(configSeed_), 0);
    drawConfigRow("Dimensions", std::to_string(configDimensionCount_), 1);

    ofSetColor(220);
    drawText("Cells per axis", axisArea.x, axisArea.y - 16.0f, bodyFont_);

    ofSetColor(28);
    ofDrawRectangle(axisArea);

    for (int axis = 0; axis < configDimensionCount_; ++axis) {
        const ofRectangle row = configAxisRowRect(axis);
        if (row.y >= axisArea.y && row.y + row.height <= axisArea.y + axisArea.height) {
            drawConfigAxisRow(axis);
        }
    }

    drawScrollbar(axisArea, configAxisContentHeight(), configAxisScrollOffset_);

    const bool canGenerate = canGenerateConfig();
    const std::string cellsText = canGenerate
        ? "Total cells: " + std::to_string(configuredCellCount())
        : "Total cells: too many";
    ofSetColor(canGenerate ? ofColor(215) : ofColor(245, 115, 115));
    drawText(cellsText, screenButtonRect(4).x, screenButtonRect(4).y - 22.0f, smallFont_);

    if (!configStatus_.empty()) {
        ofSetColor(245, 115, 115);
        drawText(configStatus_, screenButtonRect(4).x, screenButtonRect(4).y - 42.0f, smallFont_);
    }

    drawButton(screenButtonRect(4), "Generate", canGenerate);
    drawButton(screenButtonRect(5), "Back", false);
}

void ofApp::drawFinishScreen() const {
    ofSetColor(240);
    drawCenteredText("Solved", {0.0f, ofGetHeight() * 0.5f - 162.0f, static_cast<float>(ofGetWidth()), 60.0f}, titleFont_);

    drawText(
        "Path length: " + std::to_string(game_.playerPath().size()),
        ofGetWidth() * 0.5f - 60.0f,
        ofGetHeight() * 0.5f - 82.0f,
        bodyFont_
    );

    drawButton(screenButtonRect(0), "Play again", true);
    drawButton(screenButtonRect(1), "Configure", false);
}

void ofApp::drawGui() const {
    const float panelX = guiX();
    const float panelHeight = ofGetHeight() - margin_ * 2.0f;
    const ofRectangle axesArea = axesAreaRect();

    ofSetColor(34);
    ofDrawRectangle(panelX, margin_, guiWidth_, panelHeight);

    ofSetColor(235);
    drawText("Axes", panelX + guiPadding_, margin_ + 24.0f, bodyFont_);

    ofSetColor(28);
    ofDrawRectangle(axesArea);

    if (game_.hasMaze()) {
        for (int axis = 0; axis < game_.maze().dim(); ++axis) {
            const ofRectangle row = axisRowRect(axis);
            if (row.y >= axesArea.y && row.y + row.height <= axesArea.y + axesArea.height) {
                drawAxisRow(axis);
            }
        }

        drawScrollbar(axesArea, axesContentHeight(), axisScrollOffset_);
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
    drawText(
        std::to_string(sliceCoord_[axis]) + " / " + std::to_string(maxValue),
        track.x,
        track.y + track.height + 15.0f,
        smallFont_
    );
}

void ofApp::drawButton(const ofRectangle& rect, const std::string& label, const bool active) const {
    ofSetColor(active ? ofColor(70, 150, 255) : ofColor(58));
    ofDrawRectRounded(rect, 6.0f);

    ofSetColor(245);
    drawCenteredText(label, rect, buttonFont_);
}

void ofApp::drawConfigRow(const std::string& label, const std::string& value, const int row) const {
    const ofRectangle base = screenButtonRect(row);
    const ofRectangle minus = configMinusButtonRect(row);
    const ofRectangle plus = configPlusButtonRect(row);
    const float textY = base.y + 25.0f;

    ofSetColor(220);
    drawText(label, base.x, textY, bodyFont_);
    drawRightAlignedText(value, minus.x - 24.0f, textY, bodyFont_);

    drawButton(minus, "-", false);
    drawButton(plus, "+", false);
}

void ofApp::drawConfigAxisRow(const int axis) const {
    const ofRectangle row = configAxisRowRect(axis);
    const ofRectangle minus = configAxisMinusButtonRect(axis);
    const ofRectangle plus = configAxisPlusButtonRect(axis);

    ofSetColor(axis % 2 == 0 ? ofColor(33) : ofColor(38));
    ofDrawRectangle(row);

    ofSetColor(220);
    const float textY = row.y + 28.0f;
    drawText("Axis " + std::to_string(axis), row.x + 16.0f, textY, bodyFont_);
    drawRightAlignedText(std::to_string(configDimensions_[axis]), minus.x - 24.0f, textY, bodyFont_);

    drawButton(minus, "-", false);
    drawButton(plus, "+", false);
}

void ofApp::drawHud() const {
    if (!game_.hasMaze()) {
        return;
    }

    const ofRectangle area = hudAreaRect();
    std::vector<std::string> lines;

    std::string selectedText = "Selected axes: ";
    const std::vector<int> axes = selectedAxes();
    for (std::size_t i = 0; i < axes.size(); ++i) {
        if (i > 0) {
            selectedText += ", ";
        }
        selectedText += std::to_string(axes[i]);
    }

    lines.push_back(selectedText);
    lines.push_back("Solved: " + std::string(game_.solved() ? "yes" : "no"));
    lines.push_back("Status: " + statusText_);
    lines.push_back("");
    lines.push_back("Slice");
    for (std::size_t axis = 0; axis < sliceCoord_.size(); ++axis) {
        lines.push_back("  axis " + std::to_string(axis) + ": " + std::to_string(sliceCoord_[axis]));
    }
    lines.push_back("");
    lines.push_back("Player");
    for (std::size_t axis = 0; axis < game_.player().size(); ++axis) {
        lines.push_back("  axis " + std::to_string(axis) + ": " + std::to_string(game_.player()[axis]));
    }
    lines.push_back("");
    lines.push_back("Goal");
    for (std::size_t axis = 0; axis < game_.goal().size(); ++axis) {
        lines.push_back("  axis " + std::to_string(axis) + ": " + std::to_string(game_.goal()[axis]));
    }

    ofSetColor(28);
    ofDrawRectangle(area);

    const float x = area.x + 10.0f;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const float y = area.y + 22.0f + i * hudLineHeight_ - hudScrollOffset_;
        if (y < area.y + 14.0f || y > area.y + area.height - 8.0f) {
            continue;
        }

        ofSetColor(225);
        drawText(lines[i], x, y, smallFont_);
    }

    drawScrollbar(area, hudContentHeight(), hudScrollOffset_);
}

void ofApp::drawText(const std::string& text, const float x, const float y, const ofTrueTypeFont& font) const {
    if (font.isLoaded()) {
        font.drawString(text, x, y);
    } else {
        ofDrawBitmapString(text, x, y);
    }
}

void ofApp::drawRightAlignedText(const std::string& text, const float rightX, const float y, const ofTrueTypeFont& font) const {
    if (font.isLoaded()) {
        const ofRectangle bounds = font.getStringBoundingBox(text, 0.0f, 0.0f);
        font.drawString(text, rightX - bounds.width, y);
    } else {
        ofDrawBitmapString(text, rightX - text.size() * 8.0f, y);
    }
}

void ofApp::drawCenteredText(const std::string& text, const ofRectangle& rect, const ofTrueTypeFont& font) const {
    if (font.isLoaded()) {
        const ofRectangle bounds = font.getStringBoundingBox(text, 0.0f, 0.0f);
        const float x = rect.x + (rect.width - bounds.width) * 0.5f;
        const float y = rect.y + (rect.height - bounds.height) * 0.5f - bounds.y;
        font.drawString(text, x, y);
    } else {
        const float x = rect.x + rect.width * 0.5f - text.size() * 4.0f;
        const float y = rect.y + rect.height * 0.5f + 5.0f;
        ofDrawBitmapString(text, x, y);
    }
}

void ofApp::drawScrollbar(const ofRectangle& area, const float contentHeight, const float scrollOffset) const {
    const float maxScroll = std::max(0.0f, contentHeight - area.height);
    if (maxScroll <= 0.0f) {
        return;
    }

    const float handleHeight = std::max(24.0f, area.height * area.height / contentHeight);
    const float handleY = area.y + (area.height - handleHeight) * (scrollOffset / maxScroll);

    ofSetColor(54);
    ofDrawRectRounded(area.x + area.width - 5.0f, area.y, 5.0f, area.height, 3.0f);
    ofSetColor(125);
    ofDrawRectRounded(area.x + area.width - 5.0f, handleY, 5.0f, handleHeight, 3.0f);
}

bool ofApp::canGenerateConfig() const {
    return configuredCellCount() <= MAX_CONFIGURED_CELLS;
}

std::size_t ofApp::configuredCellCount() const {
    std::size_t count = 1;

    for (const int size : configDimensions_) {
        if (count > MAX_CONFIGURED_CELLS / static_cast<std::size_t>(size)) {
            return MAX_CONFIGURED_CELLS + 1;
        }

        count *= static_cast<std::size_t>(size);
    }

    return count;
}

void ofApp::updateSliderFromMouse(const int axis, const float mouseX) {
    const ofRectangle track = sliderTrackRect(axis);
    const float t = ofClamp((mouseX - track.x) / track.width, 0.0f, 1.0f);
    const int maxValue = game_.maze().shape()[axis] - 1;
    sliceCoord_[axis] = static_cast<int>(std::round(t * maxValue));
}

void ofApp::clampAxisScrollOffset() {
    axisScrollOffset_ = ofClamp(axisScrollOffset_, 0.0f, maxAxisScrollOffset());
}

void ofApp::clampConfigAxisScrollOffset() {
    const float nextScrollOffset = ofClamp(configAxisScrollOffset_, 0.0f, maxConfigAxisScrollOffset());
    configAxisScrollOffset_ = ofClamp(
        std::round(nextScrollOffset / configAxisRowHeight_) * configAxisRowHeight_,
        0.0f,
        maxConfigAxisScrollOffset()
    );
}

void ofApp::clampHudScrollOffset() {
    hudScrollOffset_ = ofClamp(hudScrollOffset_, 0.0f, maxHudScrollOffset());
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

ofRectangle ofApp::screenButtonRect(const int row) const {
    if (screen_ == Screen::Configure) {
        const float panelY = 52.0f;
        const float panelWidth = std::min(760.0f, std::max(360.0f, ofGetWidth() - 160.0f));
        const float width = panelWidth - 96.0f;
        const float height = 38.0f;
        const float x = ofGetWidth() * 0.5f - width * 0.5f;
        const float panelBottom = ofGetHeight() - panelY;

        if (row == 4) {
            return {x, panelBottom - 110.0f, width, height};
        }
        if (row == 5) {
            return {x, panelBottom - 62.0f, width, height};
        }

        return {x, panelY + 96.0f + row * 54.0f, width, height};
    }

    const float width = 260.0f;
    const float height = 38.0f;
    const float x = ofGetWidth() * 0.5f - width * 0.5f;
    const float y = ofGetHeight() * 0.5f - 54.0f + row * 52.0f;

    return {x, y, width, height};
}

ofRectangle ofApp::configMinusButtonRect(const int row) const {
    const ofRectangle base = screenButtonRect(row);
    return {base.x + base.width - 84.0f, base.y + 2.0f, 34.0f, 34.0f};
}

ofRectangle ofApp::configPlusButtonRect(const int row) const {
    const ofRectangle base = screenButtonRect(row);
    return {base.x + base.width - 38.0f, base.y + 2.0f, 34.0f, 34.0f};
}

float ofApp::configAxisContentHeight() const {
    return configDimensions_.size() * configAxisRowHeight_;
}

float ofApp::maxConfigAxisScrollOffset() const {
    return std::max(0.0f, configAxisContentHeight() - configAxisAreaRect().height);
}

ofRectangle ofApp::configAxisAreaRect() const {
    const ofRectangle topRow = screenButtonRect(1);
    const ofRectangle generateButton = screenButtonRect(4);
    const float x = topRow.x;
    const float y = topRow.y + topRow.height + 66.0f;
    const float width = topRow.width;
    const float rawHeight = std::max(configAxisRowHeight_ * 3.0f, generateButton.y - y - 54.0f);
    const float height = std::floor(rawHeight / configAxisRowHeight_) * configAxisRowHeight_;

    return {x, y, width, height};
}

ofRectangle ofApp::configAxisRowRect(const int axis) const {
    const ofRectangle area = configAxisAreaRect();
    const float y = area.y + axis * configAxisRowHeight_ - configAxisScrollOffset_;

    return {area.x, y, area.width, configAxisRowHeight_};
}

ofRectangle ofApp::configAxisMinusButtonRect(const int axis) const {
    const ofRectangle row = configAxisRowRect(axis);
    return {row.x + row.width - 86.0f, row.y + 6.0f, 32.0f, 32.0f};
}

ofRectangle ofApp::configAxisPlusButtonRect(const int axis) const {
    const ofRectangle row = configAxisRowRect(axis);
    return {row.x + row.width - 44.0f, row.y + 6.0f, 32.0f, 32.0f};
}

float ofApp::guiX() const {
    return ofGetWidth() - guiWidth_ - margin_;
}

float ofApp::axesContentHeight() const {
    return axisSelected_.size() * axisRowHeight_;
}

float ofApp::maxAxisScrollOffset() const {
    return std::max(0.0f, axesContentHeight() - axesAreaRect().height);
}

float ofApp::hudContentHeight() const {
    if (!game_.hasMaze()) {
        return 0.0f;
    }

    return (9.0f + game_.maze().dim() * 3.0f) * hudLineHeight_;
}

float ofApp::maxHudScrollOffset() const {
    return std::max(0.0f, hudContentHeight() - hudAreaRect().height);
}

ofRectangle ofApp::hudAreaRect() const {
    const ofRectangle load = loadButtonRect();
    const float x = guiX() + guiPadding_;
    const float y = load.y + load.height + 18.0f;
    const float width = guiWidth_ - guiPadding_ * 2.0f;
    const float height = std::max(80.0f, ofGetHeight() - margin_ - y);

    return {x, y, width, height};
}

ofRectangle ofApp::axesAreaRect() const {
    const float x = guiX() + guiPadding_;
    const float y = margin_ + 42.0f;
    const float width = guiWidth_ - guiPadding_ * 2.0f;
    const float reservedHeight = 360.0f;
    const float height = std::max(120.0f, ofGetHeight() - y - margin_ - reservedHeight);

    return {x, y, width, height};
}

ofRectangle ofApp::axisRowRect(const int axis) const {
    const ofRectangle axesArea = axesAreaRect();
    const float y = axesArea.y + axis * axisRowHeight_ - axisScrollOffset_;

    return {axesArea.x, y, axesArea.width, axisRowHeight_};
}

ofRectangle ofApp::axisButtonRect(const int axis) const {
    const ofRectangle row = axisRowRect(axis);
    const float x = row.x;
    const float y = row.y + 4.0f;
    return {x, y, 44.0f, 30.0f};
}

ofRectangle ofApp::sliderTrackRect(const int axis) const {
    const ofRectangle button = axisButtonRect(axis);
    const float x = button.x + button.width + 14.0f;
    const float width = guiWidth_ - guiPadding_ * 2.0f - button.width - 14.0f;
    return {x, button.y + 2.0f, width, 18.0f};
}

ofRectangle ofApp::solutionToggleRect() const {
    const ofRectangle axesArea = axesAreaRect();
    const float y = axesArea.y + axesArea.height + 16.0f;
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
