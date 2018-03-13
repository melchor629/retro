#ifndef retroeditor
#error "Cannot include this file standalone, is a private implementation for the editor"
#endif

class CreateSpritesFileScreen: public Level {
protected:

    string compositing;
    string fileName;
    bool redraw = true;
    
    virtual void setup() override {
        this->fileName = "";
        this->redraw = true;
    }
    
    virtual void update(float) override {}
    
    virtual void mustRedraw() override { redraw = true; }
    
    void createFile() {
        ga.endInputText();
        transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);
        game<Editor>().changeToSpriteEditor(fileName + ".spr");
    }
    
    virtual void mouseUp(int button, int) override {
        const auto size = ga.canvasSize();
        const Frame createButtonFrame = {{ size.x - ga.sizeOfText("Create").x - 2, size.y - 6 - 1 }, { ga.sizeOfText("Create").x + 1, 6 }};
        const Frame cancelButtonFrame = {{ size.x - ga.sizeOfText("Create").x - ga.sizeOfText("Cancel").x - 4, size.y - 6 - 1 }, { ga.sizeOfText("Cancel").x + 1, 6 }};
        
        if(button == SDL_BUTTON_LEFT) {
            if(createButtonFrame.isInside(ga.getMousePosition())) {
                createFile();
            } else if(cancelButtonFrame.isInside(ga.getMousePosition())) {
                game<Editor>().changeToSpriteFileSelector();
            }
        }
    }
    
    virtual void keyUp(int scancode) override {
        if(scancode == SDL_SCANCODE_BACKSPACE) {
            if(fileName.length() > 0) {
                redraw = true;
                auto prev = fileName.end();
                utf8::prior(prev, fileName.begin());
                fileName = fileName.substr(0, prev - fileName.begin());
            }
        } else if(scancode == SDL_SCANCODE_RETURN) {
            createFile();
        } else game<Editor>().checkChangeModeInput(ga, scancode);
    }
    
    virtual void keyText(string ch) override {
        if(utf8::distance(fileName.begin(), fileName.end()) <= 16) {
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
        ga.print("Name of the new sprites file", { 2, 2 }, { 0xFF, 0xFF, 0xFF, 0xFF });
        ga.fillRectangle({ { 3, 7 }, { ga.sizeOfText(fileName).x + ga.sizeOfText(compositing).x + 2, 6 } }, { 0x44, 0x44, 0x44, 0xFF });
        ga.print(fileName, { 4, 8 }, { 0xFA, 0xFA, 0xFA, 0xFF });
        ga.print(compositing, { 4 + ga.sizeOfText(fileName).x, 8 }, 0xB0B0B0_rgb);
        
        ga.fillRectangle({{ size.x - ga.sizeOfText("Create").x - 2, size.y - 6 - 1 }, { ga.sizeOfText("Create").x + 1, 6 }}, { 0x44, 0x44, 0x44, 0xFF });
        ga.print("Create", { size.x - ga.sizeOfText("Create").x - 1, size.y - 6 });
        ga.fillRectangle({{ size.x - ga.sizeOfText("Create").x - ga.sizeOfText("Cancel").x - 4, size.y - 6 - 1 }, { ga.sizeOfText("Cancel").x + 1, 6 }}, { 0x44, 0x44, 0x44, 0xFF });
        ga.print("Cancel", { size.x - ga.sizeOfText("Create").x - ga.sizeOfText("Cancel").x - 3, size.y - 6 });
        redraw = false;
    }
    
public:
    
    CreateSpritesFileScreen(Game &game, const char* name): Level(game, name) {}
    
};
