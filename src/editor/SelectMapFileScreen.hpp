#ifndef retroeditor
#error "Cannot include this file standalone, is a private implementation for the editor"
#endif

class SelectMapFileScreen: public Level {
protected:
    
    bool mouseOverButton;
    size_t selectedMap = 0;
    vector<string> mapsAvailable;
    bool redraw = true;
    
    virtual void setup() override {
        redraw = true;
        mapsAvailable.clear();
        selectedMap = 0;
        
        for(const std::string &path: listFiles(getGamePath())) {
            if(path.substr(path.length() - 3) == "map") {
                mapsAvailable.push_back(path);
            }
        }
    }
    
    virtual void update(float) override {}
    
    virtual void mustRedraw() override { redraw = true; }
    
    virtual void mouseUp(int button, int) override {
        const ivec2 pos = ga.getMousePosition();
        const Frame createButtonFrame = {{ ga.canvasSize().x - 6 * 4 - 2, ga.canvasSize().y - 6 - 1 }, { 6 * 4 + 1, 6 }};
        const Frame mapFileSelection = { { 5, 9 }, { ga.canvasSize().x - 5, 40 } };
        if(createButtonFrame.isInside(pos) && button == SDL_BUTTON_LEFT) {
            game<Editor>().changeToNewMapFile();
        } else if(mapFileSelection.isInside(pos) && button == SDL_BUTTON_LEFT) {
            size_t item = (pos.y - mapFileSelection.pos.y) / 8 + selectedMap / 5 * 5;
            if(item < mapsAvailable.size() && Frame{ mapFileSelection.pos + vec2 { 0, item * 8.0f }, { 2.0f + ga.sizeOfText(mapsAvailable[item]).x, 6.0f } }.isInside(pos)) {
                if(selectedMap != item) {
                    redraw = true;
                    selectedMap = item;
                } else {
                    game<Editor>().changeToMapEditor(mapsAvailable[selectedMap]);
                }
            }
        }
    }
    
    virtual void keyDown(int scancode) override {
        if(scancode == SDL_SCANCODE_UP && selectedMap > 0) {
            selectedMap -= 1;
            redraw = true;
        } else if(scancode == SDL_SCANCODE_DOWN) {
            selectedMap = glm::min(selectedMap + 1, mapsAvailable.size() - 1);
            redraw = true;
        } else if(scancode == SDL_SCANCODE_RETURN && mapsAvailable.size() > 0) {
            game<Editor>().changeToMapEditor(mapsAvailable[selectedMap]);
        } else game<Editor>().checkChangeModeInput(ga, scancode);
    }
    
    virtual bool predraw() override { return redraw; }
    
    virtual void draw() override {
        uvec2 size = ga.canvasSize();
        ga.fillRectangle({ { 0, 0 }, ga.canvasSize() }, { 0x4F, 0x5A, 0x69, 0xFF });
        
        if(mapsAvailable.size() == 0) {
            ga.print("No map created yet", { 2, 2 }, { 0xFF, 0xFF, 0xFF, 0xFF });
        } else {
            ga.print("Select one map file", { 2, 2 }, { 0xFF, 0xFF, 0xFF, 0xFF });
            size_t start = (selectedMap / 5) * 5;
            size_t end = glm::min((selectedMap / 5 + 1) * 5, mapsAvailable.size());
            size_t parts = round(mapsAvailable.size() / 5.f + 1.f);
            for(size_t i = start; i < end; i++) {
                if(i == selectedMap)
                    ga.fillRectangle({{5, 9+(i - start) * 8},{2+ga.sizeOfText(mapsAvailable[i]).x,6}}, { 0x44, 0x44, 0x44, 0xFF });
                ga.print(mapsAvailable[i], { 6, 10+(i - start) * 8 }, { 0xFA, 0xFA, 0xFA, 0xFF });
            }
            
            ga.fillRectangle({ { 2, 8 }, { 1, 40 } }, { 0xAF, 0xAF, 0xAF, 0xFF });
            ga.fillRectangle({ { 2, 8+40 / parts * start / 5 }, { 1, 40 / parts } }, { 0xFA, 0xFA, 0xFA, 0xFF });
        }
        
        ga.fillRectangle({{ size.x - ga.sizeOfText("Create").x - 2, size.y - 6 - 1 }, { ga.sizeOfText("Create").x + 1, 6 }}, { 0x44, 0x44, 0x44, 0xFF });
        ga.print("Create", { size.x - ga.sizeOfText("Create").x - 1, size.y - 6 });
        this->redraw = false;
    }
    
public:
    
    SelectMapFileScreen(Game &game, const char* name): Level(game, name) {}
    
};
