#ifndef retroeditor
#error "Cannot include this file standalone, is a private implementation for the editor"
#endif

class SpritesEditorScreen: public Level {
protected:
    
    enum { SELECT, DRAW, FILL } mode = DRAW;
    vec2 spritePos = { 0, 0 };
    size_t selectedSprite = 0;
    size_t selectedSpriteSize = 8;
    Sprites* sprites;
    bool mouseInsideCanvas = false;
    uvec2 mousePosCanvas;
    size_t selectedColor = 0;
    size_t colorPage = 0;
    size_t palettePage = 0;
    float paletteDesp = 0.0f;
    size_t spritesPage = 0;
    float spritesDesp = 0.0f;
    string notificationMessage;
    chrono::time_point<chrono::system_clock> notificationStartTime;
    bool savedPressedDone = false;
    Frame selectedRegion;
    bool selection = false;
    uint8_t drawSize = 1;
    bool copyPressedDone = false;
    struct { uint8_t* pixels = nullptr; uvec2 size; } copyBuffer;
    
    bool redraw = true;
    
    virtual void setup() override {
        this->redraw = true;
        sprites->regenerateTextures();
    }
    
    virtual void mustRedraw() override { redraw = true; }
    
    virtual void update(float) override {
        const Frame canvasFrame = { { 6, 2 }, { 32, 32 } };
        const auto pos = ga.getMousePosition();
        
        if(canvasFrame.isInside(pos)) {
            this->mouseInsideCanvas = true;
            float m = 32.0f / this->selectedSpriteSize;
            auto lastMousePosCanvas = this->mousePosCanvas;
            if(m >= 1.f) {
                this->mousePosCanvas = {
                    uint32_t((pos.x - 6) / m) * m + 6,
                    uint32_t((pos.y - 2) / m) * m + 2
                };
            } else {
                this->mousePosCanvas = ga.mpTHICC();
            }
            
            this->redraw = redraw || lastMousePosCanvas != this->mousePosCanvas;
            
            if(ga.isMousePressed(SDL_BUTTON_LEFT)) {
                vec2 leframepos = canvasFrame.pos * (m >= 1.0f ? 1.0f : 2.0f);if(m<1)m=1;
                uvec2 canvasPos = uvec2((vec2(this->mousePosCanvas) - leframepos) / m);
                if(this->mode == DRAW) {
                    vec2 lastCanvasPos = (vec2(lastMousePosCanvas) - leframepos) / m;
                    vec2 diff = vec2(canvasPos) - lastCanvasPos;
                    int32_t distance = sqrt(diff.x*diff.x + diff.y*diff.y);
                    auto sprite = (*sprites)[this->selectedSprite].size(this->selectedSpriteSize);
                    for(int32_t i = 0; i <= distance; i++) {
                        vec2 pos = vec2(lastCanvasPos) + vec2(diff) * (float(i) / float(distance+1));
                        if(sprite.frame().isInside(pos + sprite.frame().pos)) {
                            this->redraw = true;
                            for(size_t extraX = pos.x; extraX < std::min(pos.x + drawSize, float(selectedSpriteSize)); extraX++) {
                                for(size_t extraY = pos.y; extraY < std::min(pos.y + drawSize, float(selectedSpriteSize)); extraY++) {
                                    sprite.at(extraX, extraY) = this->selectedColor;
                                }
                            }
                        }
                    }
                    sprites->regenerateTextures();
                } else if(this->mode == FILL) {
                    uint8_t col = (*sprites)[this->selectedSprite].at(canvasPos.x, canvasPos.y);
                    if(col != this->selectedColor) {
                        this->redraw = true;
                        fill(canvasPos, col);
                        sprites->regenerateTextures();
                    }
                } else if(this->mode == SELECT) {
                    if(!this->selection) {
                        this->redraw = true;
                        selectedRegion.pos = selectedSpriteSize != 64 ? pos : ivec2(this->mousePosCanvas);
                        selectedRegion.size = selectedSpriteSize != 64 ? pos - ivec2(mousePosCanvas) : ivec2{ 1, 1 };
                        this->selection = true;
                    }
                }
            }
        } else if(this->mouseInsideCanvas) {
            redraw = true;
            this->mouseInsideCanvas = false;
            this->mousePosCanvas = { 0, 0 };
        }
        
        if(!notificationMessage.empty()) {
            auto now = chrono::system_clock::now();
            chrono::duration<double> diff = now - this->notificationStartTime;
            if(diff.count() >= 4.0) {
                this->notificationMessage = "";
                redraw = true;
            }
        }
    }
    
    virtual void keyDown(int scancode) override {
        game<Editor>().checkChangeModeInput(ga, scancode);
        if((ga.isModKeyPressed(KMOD_CTRL) || ga.isModKeyPressed(KMOD_GUI))) {
            if(scancode == SDL_SCANCODE_S && !this->savedPressedDone) {
                redraw = true;
                this->savedPressedDone = true;
                sprites->save();
                notificationMessage = "Saved";
                notificationStartTime = chrono::system_clock::now();
            } else if(scancode == SDL_SCANCODE_C && !this->copyPressedDone && this->selection) {
                Frame sel = selectedRegionGood();
                Sprite sprite = (*sprites)[selectedSprite].size(selectedSpriteSize);
                float m = 32.0f / this->selectedSpriteSize;
                sel.pos -= vec2{ 12, 4 };
                sel.pos /= m * 2.0f;
                sel.size /= m * 2.0f;
                if(sel.size.x < 0) { sel.pos.x += sel.size.x; sel.size.x *= -1; }
                if(sel.size.y < 0) { sel.pos.y += sel.size.y; sel.size.y *= -1; }
                size_t size = sel.size.x * sel.size.y;
                copyBuffer.pixels = (uint8_t*) realloc(copyBuffer.pixels, size);
                copyBuffer.size = sel.size;
                for(uint32_t y = sel.pos.y; y < sel.pos.y + sel.size.y; y++) {
                    uint32_t row = (y - sel.pos.y) * uint32_t(sel.size.x);
                    for(uint32_t x = sel.pos.x; x < sel.pos.x + sel.size.x; x++) {
                        copyBuffer.pixels[row + x - uint32_t(sel.pos.x)] = sprite.at(x, y);
                    }
                }
                redraw = true;
                copyPressedDone = true;
                notificationMessage = "Copied";
                notificationStartTime = chrono::system_clock::now();
            } else if(scancode == SDL_SCANCODE_V && copyBuffer.pixels != nullptr) {
                Sprite sprite = (*sprites)[selectedSprite].size(selectedSpriteSize);
                uvec2 pos { 0, 0 };
                if(mouseInsideCanvas) {
                    pos = mousePosCanvas - (selectedSpriteSize != 64 ? uvec2{ 6, 2 } : uvec2{ 12, 4 });
                    if(selectedSpriteSize == 8) pos /= 4;
                    if(selectedSpriteSize == 16)pos /= 2;
                }
                for(uint32_t y = 0; y < copyBuffer.size.y; y++) {
                    uint32_t row = y * uint32_t(copyBuffer.size.x);
                    for(uint32_t x = 0; x < copyBuffer.size.x; x++) {
                        if(pos.x + x < sprite.width && pos.y + y < sprite.height) {
                            sprite.at(pos.x + x, pos.y + y) = copyBuffer.pixels[row + x];
                        }
                    }
                }
                sprites->regenerateTextures();
                redraw = true;
            }
        }
        
        if(scancode == SDL_SCANCODE_ESCAPE && this->selection) {
            this->selection = false;
            redraw = true;
        } else if(scancode == SDL_SCANCODE_DELETE && this->selection) {
            Frame sel = selectedRegionGood();
            Sprite sprite = (*sprites)[selectedSprite].size(selectedSpriteSize);
            float m = 32.0f / this->selectedSpriteSize;
            sel.pos -= vec2{ 12, 4 };
            sel.pos /= m * 2.0f;
            sel.size /= m * 2.0f;
            if(sel.size.x < 0) { sel.pos.x += sel.size.x; sel.size.x *= -1; }
            if(sel.size.y < 0) { sel.pos.y += sel.size.y; sel.size.y *= -1; }
            for(uint32_t y = sel.pos.y; y < sel.pos.y + sel.size.y; y++) {
                for(uint32_t x = sel.pos.x; x < sel.pos.x + sel.size.x; x++) {
                    sprite.at(x, y) = 0;
                }
            }
            sprites->regenerateTextures();
            redraw = true;
        }
        
        if(this->selection) {
            //Move selection
            Sprite sprite = (*sprites)[this->selectedSprite].size(this->selectedSpriteSize);
            Frame selection = this->selectedRegionGood();
            const Frame spriteFrame = sprite.frame();
            float m = 32.0f / this->selectedSpriteSize;
            selection.pos -= vec2{ 12, 4 };
            selection.pos /= m * 2.0f;
            selection.size /= m * 2.0f;
            if(this->selectedSpriteSize == 64) m = 1.0f;
            if(scancode == SDL_SCANCODE_LEFT && selection.pos.x > 0.9f) {
                redraw = true;
                for(size_t y = selection.pos.y; y < selection.pos.y + selection.size.y; y++) {
                    for(int64_t x = selection.pos.x; x < selection.pos.x + selection.size.x; x++) {
                        sprite.at(x - 1, y) = sprite.at(x, y);
                    }
                    sprite.at(selection.pos.x + selection.size.x - 1, y) = 0;
                }
                this->selectedRegion.pos -= vec2{ m, 0 };
            } else if(scancode == SDL_SCANCODE_RIGHT && selection.pos.x + selection.size.x < spriteFrame.size.x) {
                redraw = true;
                for(size_t y = selection.pos.y; y < selection.pos.y + selection.size.y; y++) {
                    for(int64_t x = selection.pos.x + selection.size.x - 1; x >= selection.pos.x; x--) {
                        sprite.at(x + 1, y) = sprite.at(x, y);
                    }
                    sprite.at(selection.pos.x, y) = 0;
                }
                this->selectedRegion.pos += vec2{ m, 0 };
            } if(scancode == SDL_SCANCODE_UP && selection.pos.y > 0.9f) {
                redraw = true;
                for(int64_t x = selection.pos.x; x < selection.pos.x + selection.size.x; x++) {
                    for(size_t y = selection.pos.y; y < selection.pos.y + selection.size.y; y++) {
                        sprite.at(x, y - 1) = sprite.at(x, y);
                    }
                    sprite.at(x, selection.pos.y + selection.size.y - 1) = 0;
                }
                this->selectedRegion.pos -= vec2{ 0, m };
            } else if(scancode == SDL_SCANCODE_DOWN && selection.pos.y + selection.size.y < spriteFrame.size.y) {
                redraw = true;
                for(size_t x = selection.pos.x; x < selection.pos.x + selection.size.x; x++) {
                    for(int64_t y = selection.pos.y + selection.size.y - 1; y >= selection.pos.y; y--) {
                        sprite.at(x, y + 1) = sprite.at(x, y);
                    }
                    sprite.at(x, selection.pos.y) = 0;
                }
                this->selectedRegion.pos += vec2{ 0, m };
            }
            sprites->regenerateTextures();
        }
    }
    
    virtual void keyUp(int scancode) override {
        if((ga.isModKeyPressed(KMOD_CTRL) || ga.isModKeyPressed(KMOD_GUI) || scancode == SDL_SCANCODE_S) && savedPressedDone) {
            redraw = true;
            this->savedPressedDone = false;
        } else if((ga.isModKeyPressed(KMOD_CTRL) || ga.isModKeyPressed(KMOD_GUI) || scancode == SDL_SCANCODE_C) && copyPressedDone) {
            this->copyPressedDone = false;
        } else if((ga.isModKeyPressed(KMOD_CTRL) || ga.isModKeyPressed(KMOD_GUI)) && scancode == SDL_SCANCODE_R) {
            this->sprites->reload();
            this->sprites->regenerateTextures();
            this->redraw = true;
            this->notificationMessage = "Reloaded";
            this->notificationStartTime = chrono::system_clock::now();
        }
    }
    
    virtual void mouseDown(int button, int) override {
        const Frame canvasFrame = { { 6, 2 }, { 32, 32 } };
        const Frame saveFrame = { { 42, 28 }, { 7, 7 } };
        const auto pos = ga.getMousePosition();
        
        if(canvasFrame.isInside(pos)) {
            if(mode == SELECT && this->selection && ga.isMousePressed(SDL_BUTTON_LEFT)) {
                this->redraw = true;
                this->selection = false;
            }
            if(button == SDL_BUTTON_RIGHT) {
                float m = 32.0f / this->selectedSpriteSize;
                vec2 leframepos = canvasFrame.pos * (m >= 1.0f ? 1.0f : 2.0f);if(m<1)m=1;
                uvec2 canvasPos = uvec2((vec2(this->mousePosCanvas) - leframepos) / m);
                selectedColor = (*sprites)[selectedSprite].at(canvasPos.x, canvasPos.y);
                redraw = true;
            }
        } else if(!savedPressedDone && saveFrame.isInside(pos) && ga.isMousePressed(SDL_BUTTON_LEFT)) {
            redraw = true;
            this->savedPressedDone = true;
        }
    }
    
    virtual void mouseUp(int button, int) override {
        const Frame drawToolFrame = { { 42, 2 }, { 7, 7 } };
        const Frame fillToolFrame = { { 42, 11 }, { 8, 7 } };
        const Frame saveFrame = { { 42, 28 }, { 7, 7 } };
        const Frame selectToolFrame = { { 42, 20 }, { 7, 7 } };
        const Frame colorSelectionFrame = { { 54, 2 }, { 4*8, 4*8 } };
        const Frame spriteSelectionFrame = { { 2, 38 }, { 8*8, 4*8 } };
        const Frame sizeSelectionFrame = { { 5 + 4 * 16 + 9, 45 }, { 12, 3 } };
        const Frame sizeBrushFrame = { { 5 + 4 * 16 + 9, 50 }, { 12, 3 } };
        const Frame addSpritePageFrame = { { 5 + 4 * 16, 55 }, { 3, 3 } };
        const Frame goBackFrame = { { 1, 2 }, { 3, 5 } };
        const auto pos = ga.getMousePosition();
        
        if(button == SDL_BUTTON_LEFT) {
            this->redraw = true;
            if(drawToolFrame.isInside(pos)) mode = DRAW; else
                if(fillToolFrame.isInside(pos)) mode = FILL; else
                    if(selectToolFrame.isInside(pos)) mode = SELECT; else
                        
                        if(colorSelectionFrame.isInside(pos)) {
                            ivec2 elemPos = pos - ivec2(colorSelectionFrame.pos);
                            size_t elem = elemPos.x / 4 + elemPos.y / 4 * 8 + this->palettePage * 64;
                            if(elem < getPalette().size()) this->selectedColor = elem;
                        } else if(spriteSelectionFrame.isInside(pos)) {
                            ivec2 elemPos = pos - ivec2(spriteSelectionFrame.pos);
                            size_t elem = elemPos.x / 4 + elemPos.y / 4 * 16 + this->spritesPage * 16;
                            if(elem < sprites->size()) this->selectedSprite = elem;
                        } else if(sizeSelectionFrame.isInside(pos)) {
                            this->resizeSprite(pos, sizeSelectionFrame);
                        } else if(sizeBrushFrame.isInside(pos)) {
                            this->drawSize = (pos.x - sizeBrushFrame.pos.x) / 3 + 1;
                        } else if(saveFrame.isInside(pos) && this->savedPressedDone) {
                            redraw = true;
                            this->savedPressedDone = false;
                            sprites->save();
                            notificationMessage = "Saved";
                            notificationStartTime = chrono::system_clock::now();
                        } else if(addSpritePageFrame.isInside(pos)) {
                            sprites->addSpritesRow();
                            sprites->regenerateTextures();
                            redraw = true;
                        } else if(goBackFrame.isInside(pos)) {
                            game<Editor>().changeToSpriteFileSelector();
                        }
        }
    }
    
    virtual void mouseMoved(const ivec2 &pos, const vec2 &) override {
        const Frame canvasFrame = { { 6, 2 }, { 32, 32 } };
        const Frame sizeSelectionFrame = { { 5 + 4 * 16 + 9, 45 }, { 10, 3 } };
        const Frame sizeBrushFrame = { { 5 + 4 * 16 + 9, 50 }, { 12, 3 } };
        
        if(canvasFrame.isInside(pos)) {
            if(mode == SELECT && this->selection && ga.isMousePressed(SDL_BUTTON_LEFT)) {
                const auto oldFrame = selectedRegionGood();
                selectedRegion.size = (selectedSpriteSize != 64 ? pos : ga.mpTHICC()) - ivec2(selectedRegion.pos);
                redraw = selectedRegionGood() != oldFrame;
            }
        } else if(sizeSelectionFrame.isInside(pos)) {
            if(ga.isMousePressed(SDL_BUTTON_LEFT)) {
                this->resizeSprite(pos, sizeSelectionFrame);
            }
        } else if(sizeBrushFrame.isInside(pos)) {
            if(ga.isMousePressed(SDL_BUTTON_LEFT)) {
                this->redraw = true;
                this->drawSize = (pos.x - sizeBrushFrame.pos.x) / 3 + 1;
            }
        }
    }
    
    virtual void mouseWheelMoved(const ivec2 &motion) override {
        const Frame colorSelectionFrame = { { 54, 2 }, { 4*8, 4*8 } };
        const Frame spriteSelectionFrame = { { 2, 38 }, { 8*8, 4*8 } };
        if(colorSelectionFrame.isInside(ga.getMousePosition())) {
            redraw = true;
            this->paletteDesp = glm::min(glm::max(0.0f, this->paletteDesp + float(motion.y)), float(getPalette().size() / 64));
            this->palettePage = floor(this->paletteDesp);
        } else if(spriteSelectionFrame.isInside(ga.getMousePosition())) {
            redraw = true;
            this->spritesDesp = glm::min(glm::max(0.0f, this->spritesDesp + float(motion.y)), float(glm::max(0.0f, sprites->size() - 128.0f) / 16));
            this->spritesPage = floor(this->spritesDesp);
        }
    }
    
    virtual bool predraw() override { return redraw; }
    
    virtual void draw() override {
        ga.fillRectangle({ { 0, 0 }, ga.canvasSize() }, { 0x4F, 0x5A, 0x69, 0xFF });
        
        { //Draw sprites
            const size_t start = spritesPage * 16;
            const size_t end = glm::min(start + 16 * 8, sprites->size());
            const uvec2 regionPos = { 2, 38 };
            if(end < start + 8 * 16) ga.fillRectangle({ regionPos, { 8*8, 4*8 } }, { 0x34, 0x3B, 0x45, 0xFF });
            ga.fillRectangle({ regionPos, { 16 * 4, (end - start) / 16 * 4 } }, {0,0,0,255});
            for(size_t i = start; i < end; i++) {
                uvec2 pos = { ((i - start) % 16) * 4, ((i - start) / 16) * 4 };
                (*sprites)[i].draw_thicc({ 2u*(regionPos + pos), { 1, 1 } });
            }
            
            //Selection rectangle
            if(start < (selectedSprite + selectedSpriteSize / 8 * 16) && selectedSprite < end) {
                ivec2 pos = { ((this->selectedSprite - start) % 16) * 4, ((this->selectedSprite - start) / 16) * 4 };
                ga.drTHICC(
                           { 2 * (ivec2(regionPos) + pos), uvec2{ 1, 1 } * uint32_t(this->selectedSpriteSize) },
                           { 0xFF, 0xFF, 0xFF, 0xFF }
                           );
                
                //Hides the selecion rectangle if overflows (Pt. I)
                if(pos.y + this->selectedSpriteSize / 2 > 32) {
                    ga.fillRectangle({ regionPos + uvec2{ pos.x, 32 }, { this->selectedSpriteSize / 2, 2 } }, { 0x4F, 0x5A, 0x69, 0xFF });
                }
                
                //Hides the selecion rectangle if overflows (Pt. II)
                if(pos.x + this->selectedSpriteSize / 2 > 64) {
                    ga.fillRectangle({ regionPos + uvec2{ 64, pos.y }, { pos.x + this->selectedSpriteSize/2 - 64, this->selectedSpriteSize/2 } }, { 0x4F, 0x5A, 0x69, 0xFF });
                }
                
                //Hides the selection rectanble if overflows (Pt. III)
                if(pos.y < 0) {
                    ga.fillRectangle({ regionPos + uvec2{ pos.x, pos.y }, { selectedSpriteSize / 2, -pos.y } }, { 0x4F, 0x5A, 0x69, 0xFF });
                }
            }
            
            const size_t parts = glm::max(0, int32_t(sprites->size()) - 128) / 16 + 1;
            ga.fillRectangle({ { 3 + 4 * 16, 38 }, { 1, 32 } }, { 0xAF, 0xAF, 0xAF, 0xFF });
            ga.fillRectangle({ { 3 + 4 * 16, round(38.f+32.f / parts * spritesPage) }, { 1, round(32.f / parts) } }, { 0xFA, 0xFA, 0xFA, 0xFF });
            
            ga.print(to_string(this->selectedSprite+1), { 5 + 4 * 16, 39 }, { 0xFA, 0xFA, 0xFA, 0xFF });
        }
        
        {
            //Sprite editor section
            const Sprite sprite = (*sprites)[this->selectedSprite].size(selectedSpriteSize);
            const Frame spriteFrame = sprite.frame();
            ga.drawRectangle({ { 5, 1 }, { 34, 34 } }, { 0xFA, 0x40, 0x5F, 0xFF });
            if(sprite.width == spriteFrame.size.x && sprite.height == spriteFrame.size.y) {
                ga.fillRectangle({ { 6, 2 }, { 32, 32 } }, { 0, 0, 0, 0xFF });
            } else {
                const Frame visibleSpriteFrame = { { 6, 2 }, spriteFrame.size / float(selectedSpriteSize) * 32.0f };
                ga.fillRectangle(visibleSpriteFrame, { 0, 0, 0, 0xFF });
                for(size_t y = 2; y < 34; y++) {
                    for(size_t x = 6; x < 38; x++) {
                        if((x + y) % 2 && !visibleSpriteFrame.isInside({ x, y })) {
                            ga.putColor({ x, y }, { 200, 200, 200, 255 });
                        }
                    }
                }
            }
            sprite.draw({ { 6, 2 }, { 4, 4 } });
        }
        
        { //Draw tool 7x7
            const uvec2 pos { 42, 2 };
            Color color { 0xFF, 0xF1, 0xE8, 0xFF };
            if(this->mode != DRAW) color = { 0xAA, 0xAA, 0xAA, 0xFF };
            ga.drawLine(pos + uvec2{ 4, 0 }, pos + uvec2{ 0, 4 }, color);
            ga.drawLine(pos + uvec2{ 4, 1 }, pos + uvec2{ 2, 3 }, color);
            ga.drawLine(pos + uvec2{ 5, 1 }, pos + uvec2{ 2, 4 }, color);
            ga.drawLine(pos + uvec2{ 5, 2 }, pos + uvec2{ 3, 4 }, color);
            ga.drawLine(pos + uvec2{ 6, 2 }, pos + uvec2{ 2, 6 }, color);
            ga.drawLine(pos + uvec2{ 0, 5 }, pos + uvec2{ 0, 6 }, color);
            ga.putColor(pos + uvec2{ 1, 6 }, color);
        }
        
        { //Fill tool 8x7
            const uvec2 pos { 42, 11 };
            Color color { 0xFF, 0xF1, 0xE8, 0xFF };
            if(this->mode != FILL) color = { 0xAA, 0xAA, 0xAA, 0xFF };
            ga.drawLine(pos + uvec2{ 0, 4 }, pos + uvec2{ 0, 6 }, color);
            ga.drawLine(pos + uvec2{ 4, 0 }, pos + uvec2{ 7, 3 }, color);
            ga.drawLine(pos + uvec2{ 1, 3 }, pos + uvec2{ 4, 6 }, color);
            ga.drawLine(pos + uvec2{ 2, 3 }, pos + uvec2{ 5, 5 }, color);
            ga.drawLine(pos + uvec2{ 3, 3 }, pos + uvec2{ 6, 4 }, color);
            ga.drawLine(pos + uvec2{ 4, 3 }, pos + uvec2{ 6, 3 }, color);
            ga.putColor(pos + uvec2{ 4, 5 }, color);
        }
        
        { //Select tool 7x7
            const uvec2 pos { 43, 20 };
            Color color { 0xFF, 0xF1, 0xE8, 0xFF };
            if(this->mode != SELECT) color = { 0xAA, 0xAA, 0xAA, 0xFF };
            ga.drawLine(pos + uvec2{ 0, 0 }, pos + uvec2{ 0, 4 }, color);
            ga.drawLine(pos + uvec2{ 1, 0 }, pos + uvec2{ 4, 3 }, color);
            ga.drawLine(pos + uvec2{ 3, 3 }, pos + uvec2{ 1, 4 }, color);
            ga.putColor(pos + uvec2{ 3, 5 }, color);
            ga.drawLine(pos + uvec2{ 1, 1 }, pos + uvec2{ 1, 3 }, color);
            ga.drawLine(pos + uvec2{ 2, 2 }, pos + uvec2{ 2, 3 }, color);
        }
        
        { //Save action 7x7
            const uvec2 pos { 42, 28 };
            Color color = { 0xAA, 0xAA, 0xAA, 0xFF };
            if(this->savedPressedDone) color = { 0xFF, 0xF1, 0xE8, 0xFF };
            ga.drawLine(pos + uvec2{ 0, 0 }, pos + uvec2{ 0, 6 }, color);
            ga.drawLine(pos + uvec2{ 1, 6 }, pos + uvec2{ 5, 6 }, color);
            ga.drawLine(pos + uvec2{ 6, 5 }, pos + uvec2{ 6, 0 }, color);
            ga.drawLine(pos + uvec2{ 5, 0 }, pos + uvec2{ 1, 0 }, color);
            ga.drawLine(pos + uvec2{ 2, 1 }, pos + uvec2{ 2, 2 }, color);
            ga.drawLine(pos + uvec2{ 3, 2 }, pos + uvec2{ 4, 2 }, color);
            ga.putColor(pos + uvec2{ 4, 1 }, color);
            ga.drawLine(pos + uvec2{ 2, 5 }, pos + uvec2{ 4, 5 }, color);
        }
        
        { //Draw colors
            const size_t start = palettePage * 8 * 8;
            const size_t end = glm::min(start + 8 * 8, getPalette().size());
            const uvec2 regionPos = { 54, 2 };
            if(end < start + 8 * 8) ga.fillRectangle({ regionPos, { 4*8, 4*8 } }, { 0x34, 0x3B, 0x45, 0xFF });
            for(size_t i = start; i < end; i++) {
                uvec2 pos = { ((i - start) % 8) * 4, ((i - start) / 8) * 4 };
                if(i != 0 && getPalette()[i]->a != 0xFF) {
                    //Transparent background
                    for(uint32_t y = 0; y < 2; y++) {
                        for(uint32_t x = 0; x < 2; x++) {
                            ga.putColor({ regionPos + pos + uvec2{ 0 + 2*x, 0 + 2*y } }, { 200, 200, 200, 255 });
                            ga.putColor({ regionPos + pos + uvec2{ 1 + 2*x, 1 + 2*y } }, { 200, 200, 200, 255 });
                            ga.putColor({ regionPos + pos + uvec2{ 1 + 2*x, 0 + 2*y } }, { 100, 100, 100, 255 });
                            ga.putColor({ regionPos + pos + uvec2{ 0 + 2*x, 1 + 2*y } }, { 100, 100, 100, 255 });
                        }
                    }
                }
                ga.fillRectangle({ regionPos + pos, { 4, 4 }}, *getPalette()[i]);
                if(i == selectedColor) {
                    ga.drTHICC({ 2u*(regionPos + pos), { 8, 8 }}, { 0xFA, 0x40, 0x5F, 0xFF });
                }
                if(i == 0) {
                    ga.dlTHICC(2u*regionPos + uvec2{2,2}, 2u*regionPos + uvec2{5,5}, {255,255,255,255});
                    ga.dlTHICC(2u*regionPos + uvec2{5,2}, 2u*regionPos + uvec2{2,5}, {255,255,255,255});
                }
            }
            
            const size_t parts = getPalette().size() / 64 + 1;
            ga.fillRectangle({ { 87, 2 }, { 1, 32 } }, { 0xAF, 0xAF, 0xAF, 0xFF });
            ga.fillRectangle({ { 87, 2+32 / parts * palettePage }, { 1, 32 / parts } }, { 0xFA, 0xFA, 0xFA, 0xFF });
        }
        
        {
            //Size change
            const uvec2 pos = { 5 + 4 * 16, 45 };
            ga.print("x"+to_string(this->selectedSpriteSize/8), pos, { 0xFA, 0xFA, 0xFA, 0xFF });
            ga.drawLine(pos + uvec2{ 10, 1 }, pos + uvec2{ 10 + 9, 1 }, { 0xAF, 0xAF, 0xAF, 0xFF });
            if(this->selectedSpriteSize == 8) {
                ga.drawRectangle({ pos + uvec2{  9, 0 }, { 3, 3 } }, {0xFA, 0x40, 0x5F, 0xFF});
            } else if(this->selectedSpriteSize == 16) {
                ga.drawRectangle({ pos + uvec2{ 12, 0 }, { 3, 3 } }, {0xFA, 0x40, 0x5F, 0xFF});
            } else if(this->selectedSpriteSize == 32) {
                ga.drawRectangle({ pos + uvec2{ 15, 0 }, { 3, 3 } }, {0xFA, 0x40, 0x5F, 0xFF});
            } else if(this->selectedSpriteSize == 64) {
                ga.drawRectangle({ pos + uvec2{ 18, 0 }, { 3, 3 } }, {0xFA, 0x40, 0x5F, 0xFF});
            }
        }
        
        {
            //Paint size
            const uvec2 pos = { 5 + 4 * 16, 50 };
            ga.fillRectangle({ pos, { 4, 4 } }, { 0, 0, 0, 255 });
            ga.drawLine(pos + uvec2{ 10, 2 }, pos + uvec2{ 10 + 9, 2 }, { 0xAF, 0xAF, 0xAF, 0xFF });
            auto color = *(getPalette())[this->selectedColor];
            if(this->drawSize == 1) {
                ga.drTHICC({ 2u*pos + uvec2{ 3, 3 }, { 2, 2 } }, color);
                ga.drawRectangle({ pos + uvec2{  9, 1 }, { 3, 3 } }, {0xFA, 0x40, 0x5F, 0xFF});
            } else if(this->drawSize == 2) {
                ga.drTHICC({ 2u*pos + uvec2{ 2, 2 }, { 4, 4 } }, color);
                ga.drawRectangle({ pos + uvec2{ 12, 1 }, { 3, 3 } }, {0xFA, 0x40, 0x5F, 0xFF});
            } else if(this->drawSize == 3) {
                ga.drTHICC({ 2u*pos + uvec2{ 1, 1 }, { 6, 6 } }, color);
                ga.drawRectangle({ pos + uvec2{ 15, 1 }, { 3, 3 } }, {0xFA, 0x40, 0x5F, 0xFF});
            } else if(this->drawSize == 4) {
                ga.drTHICC({ 2u*pos + uvec2{ 0, 0 }, { 8, 8 } }, color);
                ga.drawRectangle({ pos + uvec2{ 18, 1 }, { 3, 3 } }, {0xFA, 0x40, 0x5F, 0xFF});
            }
        }
        
        {
            //Max sprites
            const uvec2 pos = { 5 + 4 * 16, 55 };
            ga.dlTHICC(2u*pos + uvec2{ 3, 0 }, 2u*pos + uvec2{ 3, 6 }, { 0xFA, 0xFA, 0xFA, 0xFF });
            ga.dlTHICC(2u*pos + uvec2{ 0, 3 }, 2u*pos + uvec2{ 6, 3 }, { 0xFA, 0xFA, 0xFA, 0xFF });
            ga.print(to_string(sprites->size()), pos + uvec2{ 6, 0 }, { 0xFA, 0xFA, 0xFA, 0xFF });
        }
        
        { //Go to select menu
            const uvec2 pos = { 1, 2 };
            ga.drawLine(pos + uvec2{ 0, 2 }, pos + uvec2{ 2, 0 }, { 0xFA, 0xFA, 0xFA, 0xFF});
            ga.drawLine(pos + uvec2{ 0, 2 }, pos + uvec2{ 2, 4 }, { 0xFA, 0xFA, 0xFA, 0xFF});
        }
        
        if(this->selection) {
            //Selected region
            const Color color = { 0xFA, 0xFA, 0xFA, 0xFF };
            const auto region = this->selectedRegionGood();
            if(region.size.x >= 0) {
                for(int x = region.pos.x; x < glm::min(region.pos.x + region.size.x, 76.0f); x += 2) {
                    ga.dlTHICC({ x, region.pos.y }, { x, region.pos.y }, color);
                    if(region.pos.y + region.size.y - 1 < 68)
                        ga.dlTHICC({ x + 1, region.pos.y + region.size.y - 1 }, { x + 1, region.pos.y + region.size.y - 1 }, color);
                }
            } else {
                for(int x = region.pos.x + region.size.x - 1; x < region.pos.x; x += 2) {
                    ga.dlTHICC({ x, region.pos.y }, { x, region.pos.y }, color);
                    ga.dlTHICC({ x + 1, region.pos.y + region.size.y - 1 }, { x + 1, region.pos.y + region.size.y - 1 }, color);
                }
            }
            if(region.size.y >= 0) {
                for(int y = region.pos.y; y < glm::min(region.pos.y + region.size.y, 68.0f); y += 2) {
                    ga.dlTHICC({ region.pos.x, y }, { region.pos.x, y }, color);
                    if(region.pos.x + region.size.x - 1 < 76)
                        ga.dlTHICC({ region.pos.x + region.size.x - 1, y + 1 }, { region.pos.x + region.size.x - 1, y + 1 }, color);
                }
            } else {
                for(int y = region.pos.y + region.size.y - 1; y < region.pos.y; y += 2) {
                    ga.dlTHICC({ region.pos.x, y }, { region.pos.x, y }, color);
                    ga.dlTHICC({ region.pos.x + region.size.x - 1, y + 1 }, { region.pos.x + region.size.x - 1, y + 1 }, color);
                }
            }
        }
        
        if((this->mode == DRAW || this->mode == FILL) && this->mouseInsideCanvas) {
            Color color = *getPalette()[this->selectedColor];
            float m = 32.0f / this->selectedSpriteSize;
            if(m > 0.9f) {
                uint32_t x = std::min(mousePosCanvas.x + uint32_t(drawSize * m), 6 + uint32_t(selectedSpriteSize*m)) - mousePosCanvas.x;
                uint32_t y = std::min(mousePosCanvas.y + uint32_t(drawSize * m), 2 + uint32_t(selectedSpriteSize*m)) - mousePosCanvas.y;
                ga.drTHICC({ 2u*this->mousePosCanvas, vec2{ x, y } * 2.f }, color * vec4(1, 1, 1, 0.5));
            } else {
                uint32_t x = std::min(mousePosCanvas.x + drawSize, 12 + uint32_t(selectedSpriteSize)) - mousePosCanvas.x;
                uint32_t y = std::min(mousePosCanvas.y + drawSize, 4 + uint32_t(selectedSpriteSize)) - mousePosCanvas.y;
                ga.drTHICC({ this->mousePosCanvas, vec2{ x, y } }, color * vec4(1, 1, 1, 0.5));
            }
        }
        
        if(!this->notificationMessage.empty()) {
            const auto size = ga.canvasSize();
            const auto textSize = ga.sizeOfText(this->notificationMessage);
            ga.fillRectangle({ { size.x - textSize.x - 4, 2 }, { textSize.x + 2, textSize.y + 2 } }, { 0xFA, 0x40, 0x5F, 0xFF });
            ga.print(this->notificationMessage, { size.x - textSize.x - 2, 3 }, { 0xFF, 0xFF, 0xFF, 0xFF });
        }
        
        redraw = false;
    }
    
    void fill(const uvec2 &canvasPos, uint8_t fromCol) {
        auto sprite = (*sprites)[this->selectedSprite].size(this->selectedSpriteSize);
        if(sprite.at(canvasPos.x, canvasPos.y) == fromCol) {
            sprite.at(canvasPos.x, canvasPos.y) = this->selectedColor;
            if(canvasPos.x + 1 < sprite.frame().size.x) fill(canvasPos + uvec2{ 1, 0 }, fromCol);
            if(canvasPos.x > 0) fill(canvasPos - uvec2{ 1, 0 }, fromCol);
            if(canvasPos.y + 1 < sprite.frame().size.y) fill(canvasPos + uvec2{ 0, 1 }, fromCol);
            if(canvasPos.y > 0) fill(canvasPos - uvec2{ 0, 1 }, fromCol);
        }
    }
    
    Frame selectedRegionGood() {
        Frame f;
        float m = 32.0f / this->selectedSpriteSize;
        int32_t n = m;
        if(n != 0) {
            f.pos = this->selectedRegion.pos;
            f.pos.x = uint32_t((uint32_t(f.pos.x) - 6) / n) * n + 6;
            f.pos.y = uint32_t((uint32_t(f.pos.y) - 2) / n) * n + 2;
            f.pos *= 2.0f;
            if(this->selectedRegion.size.x < 0) {
                f.size.x = ((int32_t) this->selectedRegion.size.x - n) / n * n + 1;
                f.pos.x += n*2-1;
            } else {
                f.size.x = ((int32_t) this->selectedRegion.size.x + n) / n * n;
            }
            if(this->selectedRegion.size.y < 0) {
                f.size.y = ((int32_t) this->selectedRegion.size.y - n) / n * n + 1;
                f.pos.y += n*2-1;
            } else {
                f.size.y = ((int32_t) this->selectedRegion.size.y + n) / n * n;
            }
            f.size *= 2;
        } else {
            f.pos = this->selectedRegion.pos;
            f.size = this->selectedRegion.size;
            f.size.x += 1;
            f.size.y += 1;
        }
        return f;
    }
    
    template <typename T> int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }
    
    void resizeSpriteStep(size_t i, size_t o) {
        Frame f = this->selectedRegionGood();
        size_t oldValue = 8 << o;
        size_t newValue = 8 << i;
        vec2 offset = { 6, 2 };
        if(oldValue != 64) {
            f.pos /= 2.0f;
            f.size /= 2.0f;
        } else offset *= 2;
        
        f.size -= vec2{ sgn(f.size.x) > 0 ? 1 : 0, sgn(f.size.y) > 0 ? 1 : 0 };
        if(oldValue < newValue) {
            float mult = (i - o) * 2.0f;
            f.size /= mult * (newValue == 64 && oldValue == 8 ? 1.5f : 1.0f);
            f.pos = (f.pos - offset) / mult + offset;
        } else if(oldValue > newValue) {
            float mult = (o - i) * (oldValue == 64 ? 1.0f : 2.0f);
            f.size *= mult;
            f.pos = (f.pos - offset) * mult + offset;
        }
        
        if(newValue == 64) {
            f.pos *= 2.0f;
            f.size *= 2.0f;
        } else if(oldValue == 64) {
            f.pos /= 2.0f;
            if(f.size.x > 0) {
                f.size.x /= 2.0f;
                f.size.x += 1.0f;
            } else {
                f.size.x /= 2.0f;
            }
            if(f.size.y > 0) {
                f.size.y /= 2.0f;
                f.size.y += 1.0f;
            } else {
                f.size.y /= 2.0f;
            }
        }
        this->selectedRegion = f;
        this->selectedSpriteSize = newValue;
    }
    
    void resizeSprite(const ivec2 &pos, const Frame &sizeSelectionFrame) {
        size_t oldValue = this->selectedSpriteSize;
        size_t i = (pos.x - sizeSelectionFrame.pos.x) / 3;
        size_t newValue = 8 << i;
        if(this->selection && oldValue != newValue) {
            redraw = true;
            size_t o = (oldValue >> 3) / 2;
            //Resize step by step, this way works correctly the resize of the selection
            if(o < i) {
                for(long p = o + 1; p <= long(i); p++) {
                    this->resizeSpriteStep(p, p - 1);
                }
            } else {
                for(long p = o - 1; p >= long(i); p--) {
                    this->resizeSpriteStep(p, p + 1);
                }
            }
        } else if(!this->selection) {
            redraw = true;
            this->selectedSpriteSize = newValue;
        }
    }
    
public:
    
    SpritesEditorScreen(Game &game, const char* name): Level(game, name) {}
    void setSprites(Sprites* sp) { this->sprites = sp; }
    
};
