#ifndef retroeditor
#error "Cannot include this file standalone, is a private implementation for the editor"
#endif

class CreateMapFileScreen: public Level {
protected:
    
    string fileName, compositing;
    bool redraw = true;
    size_t selectedSprite = 0;
    vector<string> spritesAvailable;
    bool focusOnInput = false;
    bool focusOnSprites = false;
    uvec2 size;
    
    virtual void setup() override {
        fileName = "";
        redraw = true;
        size = { 128, 32 };
        
        selectedSprite = 0;
        spritesAvailable.clear();
        focusOnInput  = focusOnSprites = false;
        
        for(const std::string &path: listFiles(getGamePath())) {
            if(path.substr(path.length() - 3) == "spr") {
                spritesAvailable.push_back(path);
            }
        }
    }
    
    virtual void update(float) override {}
    
    virtual void mustRedraw() override { redraw = true; }
    
    void createFile() {
        ga.endInputText();
        transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);
        Map::createMap(fileName + ".map", game(), Sprites(spritesAvailable[selectedSprite], game()), size);
        game<Editor>().changeToMapEditor(fileName + ".map");
    }
    
    virtual void mouseDown(int button, int) override {
        const Frame widthFrame { { 3 + ga.sizeOfText("WIDTH:").x + 2, 14 + 1 }, { 4 * 3, 3 } };
        const Frame heightFrame { { 3 + ga.sizeOfText("HEIGHT:").x + 2, 14 + 8 }, { 4 * 3, 3 } };
        const ivec2 pos = ga.getMousePosition();
        
        if(button == SDL_BUTTON_LEFT) {
            if(widthFrame.isInside(pos)) {
                size.x = 128 << uint32_t((pos.x - widthFrame.pos.x + 1) / 3);
                redraw = true;
            } else if(heightFrame.isInside(pos)) {
                size.y = 32 << uint32_t((pos.x - heightFrame.pos.x + 1) / 3);
                redraw = true;
            }
        }
    }
    
    virtual void mouseUp(int button, int) override {
        const auto size = ga.canvasSize();
        const auto pos = ga.getMousePosition();
        const Frame createButtonFrame = {{ size.x - ga.sizeOfText("Create").x - 2, size.y - 6 - 1 }, { ga.sizeOfText("Create").x + 1, 6 }};
        const Frame cancelButtonFrame = {{ size.x - ga.sizeOfText("Create").x - ga.sizeOfText("Cancel").x - 4, size.y - 6 - 1 }, { ga.sizeOfText("Cancel").x + 1, 6 }};
        const Frame inputTextFrame = { { 3, 7 }, { ga.sizeOfText(fileName + "_").x + 2, 6 } };
        const Frame spriteFileSelection = { { ga.canvasSize().x / 2 + 5, 9 }, { ga.canvasSize().x / 2 - 5, 40 } };
        
        if(button == SDL_BUTTON_LEFT) {
            if(createButtonFrame.isInside(pos)) {
                createFile();
            } else if(cancelButtonFrame.isInside(pos)) {
                game<Editor>().changeToMapFileSelector();
            } else if(inputTextFrame.isInside(pos)) {
                if(!focusOnInput) {
                    redraw = true;
                    focusOnInput = true;
                    focusOnSprites = false;
                    ga.startInputText({ { 3, 7 }, { ga.sizeOfText(fileName + "_").x + 2, 6 } });
                }
            } else if(spriteFileSelection.isInside(pos) && button == SDL_BUTTON_LEFT) {
                size_t item = (pos.y - spriteFileSelection.pos.y) / 8 + selectedSprite / 5 * 5;
                if(!focusOnSprites) {
                    focusOnSprites = true;
                    focusOnInput = false;
                    redraw = true;
                    ga.endInputText();
                }
                if(item < spritesAvailable.size() && Frame{ spriteFileSelection.pos + vec2{ 0, item * 8.0f }, { 2.0f + ga.sizeOfText(spritesAvailable[item]).x, 6.0f } }.isInside(pos)) {
                    if(selectedSprite != item) {
                        redraw = true;
                        selectedSprite = item;
                    }
                }
            } else {
                if(focusOnInput || focusOnSprites) {
                    redraw = true;
                    focusOnInput = focusOnSprites = false;
                    ga.endInputText();
                }
            }
        }
    }
    
    virtual void mouseMoved(const ivec2 &pos, const vec2&) override {
        const Frame widthFrame { { 3 + ga.sizeOfText("WIDTH:").x + 2, 14 + 1 }, { 10, 3 } };
        const Frame heightFrame { { 3 + ga.sizeOfText("HEIGHT:").x + 2, 14 + 8 }, { 10, 3 } };
        
        if(ga.isMousePressed(SDL_BUTTON_LEFT)) {
            if(widthFrame.isInside(pos)) {
                size.x = 128 << uint32_t((pos.x - widthFrame.pos.x + 1) / 3);
                redraw = true;
            } else if(heightFrame.isInside(pos)) {
                size.y = 32 << uint32_t((pos.x - heightFrame.pos.x + 1) / 3);
                redraw = true;
            }
        }
    }
    
    virtual void keyUp(int scancode) override {
        if(focusOnInput) {
            if(scancode == SDL_SCANCODE_BACKSPACE) {
                if(fileName.size() > 0) {
                    redraw = true;
                    auto prev = fileName.end();
                    utf8::prior(prev, fileName.begin());
                    fileName = fileName.substr(0, prev - fileName.begin());
                }
            } else if(scancode == SDL_SCANCODE_RETURN) {
                createFile();
            }
        } else if(focusOnSprites) {
            if(scancode == SDL_SCANCODE_UP && this->selectedSprite > 0) {
                selectedSprite -= 1;
                redraw = true;
            } else if(scancode == SDL_SCANCODE_DOWN) {
                selectedSprite = glm::min(selectedSprite + 1, spritesAvailable.size() - 1);
                redraw = true;
            } else if(scancode == SDL_SCANCODE_RETURN) {
                createFile();
            }
        }
        game<Editor>().checkChangeModeInput(ga, scancode);
    }
    
    virtual void keyText(string ch) override {
        if(focusOnInput && fileName.size() <= 16) {
            compositing = "";
            fileName += ch;
            redraw = true;
        }
    }
    
    virtual void keyTextEdit(string ch, int start, int length) override {
        redraw = true;
        compositing = ch;
        printf("%s %d %d\n", ch.c_str(), start, length);
    }
    
    virtual bool predraw() override { return redraw; }
    virtual void draw() override {
        const uvec2 size = ga.canvasSize();
        ga.fillRectangle({ { 0, 0 }, ga.canvasSize() }, { 0x4F, 0x5A, 0x69, 0xFF });
        ga.print("Name of the new map", { 2, 2 }, { 0xFF, 0xFF, 0xFF, 0xFF });
        if(focusOnInput) {
            ga.fillRectangle({ { 3, 7 }, { ga.sizeOfText(fileName + compositing + "_").x + 2, 6 } }, { 0x44, 0x44, 0x44, 0xFF });
        } else {
            ga.fillRectangle({ { 3, 7 }, { ga.sizeOfText(fileName + compositing + "_").x + 2, 6 } }, { 0x33, 0x33, 0x33, 0xFF });
        }
        ga.print(fileName + compositing + "_", { 4, 8 }, { 0xFA, 0xFA, 0xFA, 0xFF });
        
        if(focusOnSprites) {
            ga.fillRectangle({ { size.x / 2 + 2, 8 }, { size.x / 2 - 2, 40 } }, { 0x47, 0x51, 0x5E, 0xFF });
        }
        
        if(spritesAvailable.size() == 0) {
            ga.print("No sprites created", { 2, 2 }, { 0xFF, 0xFF, 0xFF, 0xFF });
        } else {
            ga.print("Select sprites file", { size.x / 2 + 2, 2 }, { 0xFF, 0xFF, 0xFF, 0xFF });
            size_t start = (selectedSprite / 5) * 5;
            size_t end = glm::min((selectedSprite / 5 + 1) * 5, this->spritesAvailable.size());
            size_t parts = round(spritesAvailable.size() / 5.f + 1.f);
            for(size_t i = start; i < end; i++) {
                if(i == selectedSprite)
                    ga.fillRectangle({{size.x / 2 + 5, 9+(i - start) * 8},{2+ga.sizeOfText(spritesAvailable[i]).x,6}}, { 0x44, 0x44, 0x44, 0xFF });
                ga.print(spritesAvailable[i], { size.x / 2 + 6, 10+(i - start) * 8 }, { 0xFA, 0xFA, 0xFA, 0xFF });
            }
            
            ga.fillRectangle({ { size.x / 2 + 2, 8 }, { 1, 40 } }, { 0xAF, 0xAF, 0xAF, 0xFF });
            ga.fillRectangle({ { size.x / 2 + 2, 8+40 / parts * start / 5 }, { 1, 40 / parts } }, { 0xFA, 0xFA, 0xFA, 0xFF });
        }
        
        {
            uvec2 pos { 3, 14 };
            ga.print("WIDTH:", pos, { 0xFF, 0xFF, 0xFF, 0xFF });
            ga.print(to_string(this->size.x), pos + uvec2{ ga.sizeOfText("WIDTH:").x + 2 + 10 + 2, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF });
            uint32_t x = this->size.x >> 7;
            ga.drawLine((pos + uvec2{ ga.sizeOfText("WIDTH:").x + 2, 2 }), (pos + uvec2{ ga.sizeOfText("WIDTH:").x + 2 + 9, 2 }), { 0xAF, 0xAF, 0xAF, 0xFF });
            if(x == 1) x = 0; else
                if(x == 2) x = 3; else
                    if(x == 4) x = 6; else
                        if(x == 8) x = 9;
            ga.drawRectangle({ pos + uvec2{ ga.sizeOfText("WIDTH:").x + 1+x, 1 }, { 3, 3 } }, { 0xFA, 0x40, 0x5F, 0xFF });
            ga.print("HEIGHT:", pos + uvec2{ 0, 6 }, { 0xFF, 0xFF, 0xFF, 0xFF });
            ga.print(to_string(this->size.y), pos + uvec2{ ga.sizeOfText("HEIGHT:").x + 2 + 10 + 2, 6 }, { 0xFF, 0xFF, 0xFF, 0xFF });
            uint32_t y = this->size.y >> 5;
            if(y == 1) y = 0; else
                if(y == 2) y = 3; else
                    if(y == 4) y = 6; else
                        if(y == 8) y = 9;
            ga.drawLine((pos + uvec2{ ga.sizeOfText("HEIGHT:").x + 2, 8 }), (pos + uvec2{ ga.sizeOfText("HEIGHT:").x + 2 + 9, 8 }), { 0xAF, 0xAF, 0xAF, 0xFF });
            ga.drawRectangle({ pos + uvec2{ ga.sizeOfText("HEIGHT:").x + 1+y, 7 }, { 3, 3 } }, { 0xFA, 0x40, 0x5F, 0xFF });
        }
        
        ga.fillRectangle({{ size.x - ga.sizeOfText("Create").x - 2, size.y - 6 - 1 }, { ga.sizeOfText("Create").x + 1, 6 }}, { 0x44, 0x44, 0x44, 0xFF });
        ga.print("Create", { size.x - ga.sizeOfText("Create").x - 1, size.y - 6 });
        ga.fillRectangle({{ size.x - ga.sizeOfText("Create").x - ga.sizeOfText("Cancel").x - 4, size.y - 6 - 1 }, { ga.sizeOfText("Cancel").x + 1, 6 }}, { 0x44, 0x44, 0x44, 0xFF });
        ga.print("Cancel", { size.x - ga.sizeOfText("Create").x - ga.sizeOfText("Cancel").x - 3, size.y - 6 });
        redraw = false;
    }
    
public:
    
    CreateMapFileScreen(Game &game, const char* name): Level(game, name) {}
    
};
