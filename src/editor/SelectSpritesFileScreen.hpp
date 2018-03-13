#ifndef retroeditor
#error "Cannot include this file standalone, is a private implementation for the editor"
#endif

class SelectSpritesFileScreen: public Level {
protected:
    
    bool mouseOverButton;
    size_t selectedItem = 0;
    std::vector<std::string> spritesAvailable;
    bool redraw = true;
    
    virtual void setup() override {
        this->redraw = true;
        this->spritesAvailable.clear();
        this->selectedItem = 0;
        for(const std::string &path: listFiles(getGamePath())) {
            if(path.substr(path.length() - 3) == "spr") {
                spritesAvailable.push_back(path);
            }
        }
    }
    
    virtual void update(float) override {}
    
    virtual void mustRedraw() override { redraw = true; }
    
    virtual void mouseUp(int button, int) override {
        const ivec2 pos = ga.getMousePosition();
        const Frame createButtonFrame = {{ ga.canvasSize().x - 6 * 4 - 2, ga.canvasSize().y - 6 - 1 }, { 6 * 4 + 1, 6 }};
        const Frame spriteFileSelection = { { 5, 9 }, { ga.canvasSize().x, 40 } };
        if(createButtonFrame.isInside(pos) && button == SDL_BUTTON_LEFT) {
            game<Editor>().changeToNewSpriteFile();
        }
        
        if(spriteFileSelection.isInside(pos) && button == SDL_BUTTON_LEFT) {
            size_t item = (pos.y - spriteFileSelection.pos.y) / 8 + this->selectedItem / 5 * 5;
            if(item < spritesAvailable.size() && Frame{ spriteFileSelection.pos + vec2{ 0, item * 8.0f }, { 2.0f + ga.sizeOfText(spritesAvailable[item]).x, 6.0f } }.isInside(pos)) {
                if(this->selectedItem != item) {
                    this->selectedItem = item;
                    redraw = true;
                } else {
                    game<Editor>().changeToSpriteEditor(this->spritesAvailable[item]);
                }
            }
        }
    }
    
    virtual void keyDown(int scancode) override {
        if(scancode == SDL_SCANCODE_UP && this->selectedItem > 0) {
            this->selectedItem -= 1;
            this->redraw = true;
        } else if(scancode == SDL_SCANCODE_DOWN) {
            this->selectedItem = glm::min(this->selectedItem + 1, this->spritesAvailable.size() - 1);
            this->redraw = true;
        } else if(scancode == SDL_SCANCODE_RETURN) {
            game<Editor>().changeToSpriteEditor(this->spritesAvailable[this->selectedItem]);
        } else game<Editor>().checkChangeModeInput(ga, scancode);
    }
    
    virtual bool predraw() override { return redraw; }
    
    virtual void draw() override {
        uvec2 size = ga.canvasSize();
        ga.fillRectangle({ { 0, 0 }, ga.canvasSize() }, { 0x4F, 0x5A, 0x69, 0xFF });
        if(spritesAvailable.size() == 0) {
            ga.print("No sprites file created yet", { 2, 2 }, { 0xFF, 0xFF, 0xFF, 0xFF });
        } else {
            ga.print("Select one sprites file", { 2, 2 }, { 0xFF, 0xFF, 0xFF, 0xFF });
            size_t start = (this->selectedItem / 5) * 5;
            size_t end = glm::min((this->selectedItem / 5 + 1) * 5, this->spritesAvailable.size());
            size_t parts = round(this->spritesAvailable.size() / 5.f + 1.f);
            for(size_t i = start; i < end; i++) {
                if(i == this->selectedItem)
                    ga.fillRectangle({{5, 9+(i - start) * 8},{2+ga.sizeOfText(spritesAvailable[i]).x,6}}, { 0x44, 0x44, 0x44, 0xFF });
                ga.print(spritesAvailable[i], { 6, 10+(i - start) * 8 }, { 0xFA, 0xFA, 0xFA, 0xFF });
            }
            
            ga.fillRectangle({ { 2, 8 }, { 1, 40 } }, { 0xAF, 0xAF, 0xAF, 0xFF });
            ga.fillRectangle({ { 2, 8+40 / parts * start / 5 }, { 1, 40 / parts } }, { 0xFA, 0xFA, 0xFA, 0xFF });
        }
        
        ga.fillRectangle({{ size.x - ga.sizeOfText("Create").x - 2, size.y - 6 - 1 }, { ga.sizeOfText("Create").x + 1, 6 }}, { 0x44, 0x44, 0x44, 0xFF });
        ga.print("Create", { size.x - ga.sizeOfText("Create").x - 1, size.y - 6 });
        this->redraw = false;
    }
    
public:
    
    SelectSpritesFileScreen(Game &game, const char* name): Level(game, name) {}
    
};
