#pragma once

#include "mazeGame.h"
#include "mazeRenderer.h"
#include "ofMain.h"

class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;

		void keyPressed(int key) override;
		void keyReleased(int key) override;
		void mouseMoved(int x, int y ) override;
		void mouseDragged(int x, int y, int button) override;
		void mousePressed(int x, int y, int button) override;
		void mouseReleased(int x, int y, int button) override;
		void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
		void mouseEntered(int x, int y) override;
		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
		void dragEvent(ofDragInfo dragInfo) override;
		void gotMessage(ofMessage msg) override;

private:
        void setupNewGame();
        void prepareDefaultGame();
        void resetAxisUi();

        std::vector<int> selectedAxes() const;
        bool hasDrawableSlice() const;
        bool tryMove(int axis, bool positiveDirection);
        void syncHiddenAxesToPlayer();

        void drawGui() const;
        void drawAxisRow(int axis) const;
        void drawButton(const ofRectangle& rect, const std::string& label, bool active) const;
        void drawHud() const;

        void updateSliderFromMouse(int axis, float mouseX);
        void clampAxisScrollOffset();
        void saveMaze();
        void loadMaze();

        float guiX() const;
        float axesContentHeight() const;
        float maxAxisScrollOffset() const;
        ofRectangle axesAreaRect() const;
        ofRectangle axisRowRect(int axis) const;
        ofRectangle axisButtonRect(int axis) const;
        ofRectangle sliderTrackRect(int axis) const;
        ofRectangle solutionToggleRect() const;
        ofRectangle playerVisibleToggleRect() const;
        ofRectangle saveButtonRect() const;
        ofRectangle loadButtonRect() const;

        MazeGame game_;
        MazeRenderer2D renderer_;
        Maze::Coord sliceCoord_;
        std::vector<bool> axisSelected_;

        int draggedSliderAxis_ = -1;
        float axisScrollOffset_ = 0.0f;
        bool showSolutionPath_ = false;
        bool keepPlayerVisible_ = true;
        std::string statusText_;

        float margin_ = 36.0f;
        float guiWidth_ = 280.0f;
        float guiPadding_ = 18.0f;
        float axisRowHeight_ = 46.0f;
};
