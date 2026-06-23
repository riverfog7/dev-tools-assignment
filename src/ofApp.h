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
        enum class Screen {
            Start,
            Configure,
            Playing,
            Finished
        };

        void setupNewGame();
        void prepareDefaultGame();
        void resetAxisUi();
        void syncConfigDimensions();
        void loadFonts();

        std::vector<int> selectedAxes() const;
        bool hasDrawableSlice() const;
        bool tryMove(int axis, bool positiveDirection);
        void syncHiddenAxesToPlayer();

        void drawGameScreen();
        void drawStartScreen() const;
        void drawConfigScreen() const;
        void drawFinishScreen() const;
        void drawGui() const;
        void drawAxisRow(int axis) const;
        void drawButton(const ofRectangle& rect, const std::string& label, bool active) const;
        void drawConfigRow(const std::string& label, const std::string& value, int row) const;
        void drawConfigAxisRow(int axis) const;
        void drawHud() const;
        void drawText(const std::string& text, float x, float y, const ofTrueTypeFont& font) const;
        void drawRightAlignedText(const std::string& text, float rightX, float y, const ofTrueTypeFont& font) const;
        void drawCenteredText(const std::string& text, const ofRectangle& rect, const ofTrueTypeFont& font) const;
        void drawScrollbar(const ofRectangle& area, float contentHeight, float scrollOffset) const;

        bool canGenerateConfig() const;
        std::size_t configuredCellCount() const;
        void updateSliderFromMouse(int axis, float mouseX);
        void clampAxisScrollOffset();
        void clampConfigAxisScrollOffset();
        void clampHudScrollOffset();
        void saveMaze();
        void loadMaze();

        ofRectangle screenButtonRect(int row) const;
        ofRectangle configMinusButtonRect(int row) const;
        ofRectangle configPlusButtonRect(int row) const;
        float configAxisContentHeight() const;
        float maxConfigAxisScrollOffset() const;
        ofRectangle configAxisAreaRect() const;
        ofRectangle configAxisRowRect(int axis) const;
        ofRectangle configAxisMinusButtonRect(int axis) const;
        ofRectangle configAxisPlusButtonRect(int axis) const;
        float guiX() const;
        float axesContentHeight() const;
        float maxAxisScrollOffset() const;
        float hudContentHeight() const;
        float maxHudScrollOffset() const;
        ofRectangle hudAreaRect() const;
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

        Screen screen_ = Screen::Start;
        int draggedSliderAxis_ = -1;
        float axisScrollOffset_ = 0.0f;
        float configAxisScrollOffset_ = 0.0f;
        float hudScrollOffset_ = 0.0f;
        bool showSolutionPath_ = false;
        bool keepPlayerVisible_ = true;
        unsigned int configSeed_ = 123;
        int configDimensionCount_ = 4;
        std::vector<int> configDimensions_ = {10, 10, 10, 10};
        std::string configStatus_;
        std::string statusText_;

        ofTrueTypeFont titleFont_;
        ofTrueTypeFont bodyFont_;
        ofTrueTypeFont smallFont_;
        ofTrueTypeFont buttonFont_;

        float margin_ = 36.0f;
        float guiWidth_ = 320.0f;
        float guiPadding_ = 18.0f;
        float axisRowHeight_ = 46.0f;
        float configAxisRowHeight_ = 44.0f;
        float hudLineHeight_ = 19.0f;
};
