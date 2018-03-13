#ifndef retroeditor
#error "Cannot include this file standalone, is a private implementation for the editor"
#endif

class MapEditorScreen: public Level {
    
protected:
    bool redraw = false;
    Map* map = nullptr;
    
    size_t spritesPage = 0;
    uint8_t selectedSprite = 0;
    
    bool savedPressedDone = false;
    
    string notificationMessage;
    chrono::time_point<chrono::system_clock> notificationStartTime;
    
    enum { DRAW, FILL, PAN, SELECT, RUBBER } mode = DRAW;
    
    bool mouseInsideCanvas = false;
    uvec2 mouseCanvasPos;
    
    ivec2 mapPosition = { 0, 0 };
    vec2 mapPosAccum = { 0, 0 };
    
    bool hasSelection = false;
    Frame selection;
    
    bool copyPressedDone = false;
    struct { uint8_t* pixels = nullptr; uvec2 size; } copyBuffer;
    
    virtual void setup() override {
        redraw = true;
        map->regenerateTextures();
    }
    
    virtual void update(float) override {
        if(!notificationMessage.empty()) {
            auto now = chrono::system_clock::now();
            chrono::duration<double> diff = now - this->notificationStartTime;
            if(diff.count() >= 4.0) {
                this->notificationMessage = "";
                redraw = true;
            }
        }
    }
    
    virtual void mustRedraw() override { redraw = true; }
    
    virtual void mouseDown(int button, int) override {
        const Frame canvasPos { { 0, 0 }, { 128, 52 } };
        const ivec2 pos = ga.getMousePosition();
        if(button == SDL_BUTTON_LEFT) {
            if(mode == DRAW && canvasPos.isInside(pos)) {
                ivec2 ppos = -mapPosition + ivec2(mouseCanvasPos / 8u);
                map->at(ppos.x, ppos.y) = selectedSprite + 1;
                map->regenerateTextures();
                redraw = true;
            } else if(mode == FILL && canvasPos.isInside(pos)) {
                ivec2 ppos = -mapPosition + ivec2(mouseCanvasPos / 8u);
                if(map->at(ppos.x, ppos.y) != selectedSprite + 1) {
                    redraw = true;
                    fill(ppos, map->at(ppos.x, ppos.y));
                    map->regenerateTextures();
                }
            } else if(mode == RUBBER && canvasPos.isInside(pos)) {
                ivec2 ppos = -mapPosition + ivec2(mouseCanvasPos / 8u);
                if(map->at(ppos.x, ppos.y) != 0) {
                    map->at(ppos.x, ppos.y) = 0;
                    map->regenerateTextures();
                    redraw = true;
                }
            } else if(mode == SELECT && canvasPos.isInside(pos)) {
                ivec2 ppos = -mapPosition + ivec2(mouseCanvasPos / 8u);
                hasSelection = true;
                selection.pos = ppos;
                selection.size = { 1, 1 };
                redraw = true;
            }
        }
    }
    
    virtual void mouseUp(int button, int) override {
        const Frame drawToolFrame { { 72, 56 }, { 7, 7 } };
        const Frame fillToolFrame { { 80, 56 }, { 8, 7 } };
        const Frame selectToolFrame { { 89, 56 }, { 7, 7 } };
        const Frame panToolFrame { { 96, 56 }, { 7, 7 } };
        const Frame rubberButtonFrame { { 104, 56 }, { 7, 7 } };
        const Frame closeButtonFrame { { 112, 56 }, { 7, 7 } };
        const Frame spriteSelectionFrame { { 0, 56 }, { 64, 32 } };
        const Frame canvasPos { { 0, 0 }, { ga.canvasSize().x, ga.canvasSize().y - 16 } };
        const ivec2 pos = ga.getMousePosition();
        
        if(button == SDL_BUTTON_LEFT) {
            if(drawToolFrame.isInside(pos)) {
                redraw = true;
                mode = DRAW;
            } else if(fillToolFrame.isInside(pos)) {
                redraw = true;
                mode = FILL;
            } else if(selectToolFrame.isInside(pos)) {
                redraw = true;
                mode = SELECT;
            } else if(panToolFrame.isInside(pos)) {
                redraw = true;
                mode = PAN;
            } else if(rubberButtonFrame.isInside(pos)) {
                redraw = true;
                mode = RUBBER;
            } else if(closeButtonFrame.isInside(pos)) {
                game<Editor>().changeToMapFileSelector();
            } else if(spriteSelectionFrame.isInside(pos)) {
                ivec2 elemPos = pos - ivec2(spriteSelectionFrame.pos);
                size_t elem = elemPos.x / 4 + elemPos.y / 4 * 16 + spritesPage * 16;
                if(elem < map->getSprites()->size()) {
                    selectedSprite = elem;
                    redraw = true;
                }
            }
        } else if(button == SDL_BUTTON_RIGHT) {
            if(canvasPos.isInside(pos)) {
                ivec2 ppos = -mapPosition + ivec2(mouseCanvasPos / 8u);
                if(map->at(ppos.x, ppos.y) != 0) {
                    selectedSprite = map->at(ppos.x, ppos.y) - 1;
                }
                redraw = true;
            }
        }
    }
    
    virtual void mouseMoved(const ivec2 &pos, const vec2 &desp) override {
        const Frame canvasPos { { 0, 0 }, { ga.canvasSize().x, ga.canvasSize().y - 20 } };
        
        if(canvasPos.isInside(pos)) {
            mouseInsideCanvas = true;
            uvec2 cpos = pos / 8 * 8;
            if(cpos != mouseCanvasPos) {
                redraw = true;
                auto lastPos = mouseCanvasPos / 8u;
                mouseCanvasPos = cpos;
                
                if(ga.isMousePressed(SDL_BUTTON_LEFT) && (mode == DRAW || mode == RUBBER)) {
                    ivec2 ppos1 = -mapPosition + ivec2(mouseCanvasPos / 8u);
                    ivec2 ppos2 = -mapPosition + ivec2(lastPos);
                    vec2 diff = (ppos1 - ppos2);
                    int32_t distance = sqrt(diff.x*diff.x + diff.y*diff.y);
                    for(int32_t i = 0; i <= distance; i++) {
                        vec2 paintPos = vec2(lastPos) + diff * (float(i) / float(distance + 1));
                        map->at(paintPos.x, paintPos.y) = mode == DRAW ? selectedSprite + 1 : 0;
                    }
                    map->regenerateTextures();
                    static int ixD = 0;
                    printf("%d xD\n", ixD++);
                } else if(ga.isMousePressed(SDL_BUTTON_LEFT) && mode == PAN) {
                    doMoveMap(desp);
                } else if(ga.isMousePressed(SDL_BUTTON_LEFT) && mode == SELECT && hasSelection) {
                    vec2 ppos1 = -mapPosition + ivec2(mouseCanvasPos / 8u);
                    if(ppos1.x >= 0 && ppos1.y >= 0 && ppos1.x < map->getSize().x && ppos1.y < map->getSize().y) {
                        selection.size = ppos1 - selection.pos;
                        if(selection.size.x >= 0) selection.size.x += 1.0f; else selection.size.x -= 1.0f;
                        if(selection.size.y >= 0) selection.size.y += 1.0f; else selection.size.y -= 1.0f;
                    }
                }
            }
        } else if(mouseInsideCanvas) {
            redraw = true;
            mouseInsideCanvas = false;
        }
    }
    
    virtual void mouseWheelMoved(const ivec2 &motion) override {
        const Frame canvasPos { { 0, 0 }, { ga.canvasSize().x, ga.canvasSize().y - 16 } };
        const Frame spriteSelection { { 0, 56 }, { 16 * 4, 4 * 4 } };
        if(canvasPos.isInside(ga.getMousePosition())) {
            if(ga.isModKeyPressed(KMOD_SHIFT)) {
                doMoveMap(vec2{ -motion.y, motion.x });
            } else {
                doMoveMap(vec2{ motion.x, -motion.y });
            }
            redraw = true;
        } else if(spriteSelection.isInside(ga.getMousePosition())) {
            if(motion.y > 0) spritesPage = std::min(spritesPage + size_t(1), (map->getSprites()->size() - 64) / 16);
            else if(motion.y < 0 && spritesPage > 0) spritesPage--;
            redraw = true;
        }
    }
    
    virtual void keyDown(int scancode) override {
        game<Editor>().checkChangeModeInput(ga, scancode);
        if(ga.isModKeyPressed(KMOD_CTRL) || ga.isModKeyPressed(KMOD_GUI)) {
            if(scancode == SDL_SCANCODE_S) {
                redraw = true;
                savedPressedDone = false;
                map->save();
                notificationMessage = "Saved";
                notificationStartTime = chrono::system_clock::now();
            } else if(scancode == SDL_SCANCODE_R) {
                redraw = true;
                map->reload();
                map->regenerateTextures();
                notificationMessage = "Reloaded";
                notificationStartTime = chrono::system_clock::now();
            } else if(scancode == SDL_SCANCODE_C && hasSelection && !copyPressedDone) {
                if(selection.size.x < 0) {
                    selection.pos.x += selection.size.x + 1;
                    selection.size.x *= -1.0f;
                }
                if(selection.size.y < 0) {
                    selection.pos.y += selection.size.y + 1;
                    selection.size.y *= -1.0f;
                }
                copyBuffer.pixels = (uint8_t*) realloc(copyBuffer.pixels, selection.size.x * selection.size.y);
                copyBuffer.size = selection.size;
                for(uint32_t y = selection.pos.y; y < selection.pos.y + selection.size.y; y++) {
                    uint32_t row = (y - selection.pos.y) * uint32_t(selection.size.x);
                    for(uint32_t x = selection.pos.x; x < selection.pos.x + selection.size.x; x++) {
                        copyBuffer.pixels[row + x - uint32_t(selection.pos.x)] = map->at(x, y);
                    }
                }
                redraw = true;
                copyPressedDone = true;
                notificationMessage = "Copied";
                notificationStartTime = chrono::system_clock::now();
            } else if(scancode == SDL_SCANCODE_V && copyBuffer.pixels != nullptr) {
                ivec2 pos { 0, 0 };
                if(mouseInsideCanvas) {
                    pos = -mapPosition + ivec2(mouseCanvasPos / 8u);
                }
                if(0 <= pos.x && pos.x + copyBuffer.size.x <= map->getSize().x && 0 <= pos.y && pos.y + copyBuffer.size.y <= map->getSize().y) {
                    for(uint32_t y = 0; y < copyBuffer.size.y; y++) {
                        uint32_t row = y * uint32_t(copyBuffer.size.x);
                        for(uint32_t x = 0; x < copyBuffer.size.x; x++) {
                            map->at(pos.x + x, pos.y + y) = copyBuffer.pixels[row + x];
                        }
                    }
                    map->regenerateTextures();
                    redraw = true;
                }
            }
        }
        
        if(hasSelection) {
            if(selection.size.x < 0) {
                selection.pos.x += selection.size.x + 1;
                selection.size.x *= -1.0f;
            }
            if(selection.size.y < 0) {
                selection.pos.y += selection.size.y + 1;
                selection.size.y *= -1.0f;
            }
            if(scancode == SDL_SCANCODE_LEFT && selection.pos.x > 0.9f) {
                redraw = true;
                for(size_t y = selection.pos.y; y < selection.pos.y + selection.size.y; y++) {
                    for(int64_t x = selection.pos.x; x < selection.pos.x + selection.size.x; x++) {
                        map->at(x - 1, y) = map->at(x, y);
                    }
                    map->at(selection.pos.x + selection.size.x - 1, y) = 0;
                }
                selection.pos -= vec2{ 1, 0 };
                map->regenerateTextures();
            } else if(scancode == SDL_SCANCODE_RIGHT && selection.pos.x + selection.size.x < map->getSize().x) {
                redraw = true;
                for(size_t y = selection.pos.y; y < selection.pos.y + selection.size.y; y++) {
                    for(int64_t x = selection.pos.x + selection.size.x - 1; x >= selection.pos.x; x--) {
                        map->at(x + 1, y) = map->at(x, y);
                    }
                    map->at(selection.pos.x, y) = 0;
                }
                selection.pos += vec2{ 1, 0 };
                map->regenerateTextures();
            } else if(scancode == SDL_SCANCODE_UP && selection.pos.y > 0.9f) {
                redraw = true;
                for(int64_t x = selection.pos.x; x < selection.pos.x + selection.size.x; x++) {
                    for(size_t y = selection.pos.y; y < selection.pos.y + selection.size.y; y++) {
                        map->at(x, y - 1) = map->at(x, y);
                    }
                    map->at(x, selection.pos.y + selection.size.y - 1) = 0;
                }
                selection.pos -= vec2{ 0, 1 };
                map->regenerateTextures();
            } else if(scancode == SDL_SCANCODE_DOWN && selection.pos.y + selection.size.y < map->getSize().y) {
                redraw = true;
                for(size_t x = selection.pos.x; x < selection.pos.x + selection.size.x; x++) {
                    for(int64_t y = selection.pos.y + selection.size.y - 1; y >= selection.pos.y; y--) {
                        map->at(x, y + 1) = map->at(x, y);
                    }
                    map->at(x, selection.pos.y) = 0;
                }
                selection.pos += vec2{ 0, 1 };
                map->regenerateTextures();
            } else if(scancode == SDL_SCANCODE_ESCAPE) {
                hasSelection = false;
                redraw = true;
            } else if(scancode == SDL_SCANCODE_DELETE) {
                for(uint32_t y = selection.pos.y; y < selection.pos.y + selection.size.y; y++) {
                    for(uint32_t x = selection.pos.x; x < selection.pos.x + selection.size.x; x++) {
                        map->at(x, y) = 0;
                    }
                }
                redraw = true;
                map->regenerateTextures();
            }
        }
    }
    
    virtual bool predraw() override { return redraw; }
    
    virtual void draw() override {
        ga.fillRectangle({ { 0, 0 }, { 128, 52 } }, { 0, 0, 0, 0xFF });
        ga.fillRectangle({ { 0, 52 }, { 128, 4 } }, { 0x4F, 0x5A, 0x69, 0xFF });
        ga.fillRectangle({ { 64, 56 }, { 64, 16 } }, { 0x4F, 0x5A, 0x69, 0xFF });
        
        { //Draw map
            uvec2 size { 128, 52 };
            if(mapPosition.x > 0) {
                size.x = 128 - mapPosition.x * 8;
                ga.fillRectangle({ { 0, 0 }, { mapPosition.x * 8, 52 } }, { 0x4F, 0x5A, 0x69, 0xFF });
            } else if(mapPosition.x < -int32_t(map->getSize().x - 16)) {
                int32_t diff = (map->getSize().x + mapPosition.x) * 8;
                size.x = diff;
                ga.fillRectangle({ { size.x, 0 }, { diff, 52 } }, { 0x4F, 0x5A, 0x69, 0xFF });
            }
            if(mapPosition.y > 0) {
                size.y = 52 - mapPosition.y * 8;
                ga.fillRectangle({ { 0, 0 }, { 128, mapPosition.y * 8 } }, { 0x4F, 0x5A, 0x69, 0xFF });
            } else if(mapPosition.y < -int32_t(map->getSize().y - 6)) {
                int32_t diff = (map->getSize().y + mapPosition.y) * 8;
                size.y = diff;
                ga.fillRectangle({ { 0, size.y }, { 128, diff } }, { 0x4F, 0x5A, 0x69, 0xFF });
            }
            map->draw({ mapPosition * 8, size });
        }
        
        { //Draw sprites
            const Sprites* sprites = map->getSprites();
            const size_t start = spritesPage * 16;
            const size_t end = glm::min(start + 16 * 4, sprites->size());
            const uvec2 regionPos = { 0, 56 };
            if(end < start + 4 * 16) ga.fillRectangle({ regionPos, { 8*8, 4*8 } }, { 0x34, 0x3B, 0x45, 0xFF });
            ga.fillRectangle({ regionPos, { 16 * 4, (end - start) / 16 * 4 } }, {0,0,0,255});
            for(size_t i = start; i < end; i++) {
                uvec2 pos = { ((i - start) % 16) * 4, ((i - start) / 16) * 4 };
                (*sprites)[i].draw_thicc({ 2u*(regionPos + pos), { 1, 1 } });
            }
            
            //Selection rectangle
            if(start < (selectedSprite + 16u) && selectedSprite < end) {
                ivec2 pos = { ((selectedSprite - start) % 16) * 4, ((selectedSprite - start) / 16) * 4 };
                ga.drTHICC(
                           { 2 * (ivec2(regionPos) + pos), uvec2{ 8, 8 } },
                           { 0xFF, 0xFF, 0xFF, 0xFF }
                           );
            }
            
            const size_t parts = glm::max(0, int32_t(sprites->size()) - 64) / 16 + 1;
            ga.fillRectangle({ regionPos + uvec2{ 4 * 16 + 1, 0 }, { 1, 32 } }, { 0xAF, 0xAF, 0xAF, 0xFF });
            ga.fillRectangle({ regionPos + uvec2{ 4 * 16 + 1, round(32.f / parts / parts * spritesPage) }, { 1, round(32.f / parts) } }, { 0xFA, 0xFA, 0xFA, 0xFF });
            
            ga.print(to_string(this->selectedSprite+1), regionPos + uvec2{ 4 * 16 + 3, 11 }, { 0xFA, 0xFA, 0xFA, 0xFF });
        }
        
        { //Draw tool 7x7
            const uvec2 pos { 72, 56 };
            Color color = 0xFFF1E8_rgb;
            if(mode != DRAW) color = { 0xAA, 0xAA, 0xAA, 0xFF };
            ga.drawLine(pos + uvec2{ 4, 0 }, pos + uvec2{ 0, 4 }, color);
            ga.drawLine(pos + uvec2{ 4, 1 }, pos + uvec2{ 2, 3 }, color);
            ga.drawLine(pos + uvec2{ 5, 1 }, pos + uvec2{ 2, 4 }, color);
            ga.drawLine(pos + uvec2{ 5, 2 }, pos + uvec2{ 3, 4 }, color);
            ga.drawLine(pos + uvec2{ 6, 2 }, pos + uvec2{ 2, 6 }, color);
            ga.drawLine(pos + uvec2{ 0, 5 }, pos + uvec2{ 0, 6 }, color);
            ga.putColor(pos + uvec2{ 1, 6 }, color);
        }
        
        { //Fill tool 8x7
            const uvec2 pos { 80, 56 };
            Color color { 0xFF, 0xF1, 0xE8, 0xFF };
            if(mode != FILL) color = { 0xAA, 0xAA, 0xAA, 0xFF };
            ga.drawLine(pos + uvec2{ 0, 4 }, pos + uvec2{ 0, 6 }, color);
            ga.drawLine(pos + uvec2{ 4, 0 }, pos + uvec2{ 7, 3 }, color);
            ga.drawLine(pos + uvec2{ 1, 3 }, pos + uvec2{ 4, 6 }, color);
            ga.drawLine(pos + uvec2{ 2, 3 }, pos + uvec2{ 5, 5 }, color);
            ga.drawLine(pos + uvec2{ 3, 3 }, pos + uvec2{ 6, 4 }, color);
            ga.drawLine(pos + uvec2{ 4, 3 }, pos + uvec2{ 6, 3 }, color);
            ga.putColor(pos + uvec2{ 4, 5 }, color);
        }
        
        { //Select tool 7x7
            const uvec2 pos { 89, 56 };
            Color color { 0xFF, 0xF1, 0xE8, 0xFF };
            if(mode != SELECT) color = { 0xAA, 0xAA, 0xAA, 0xFF };
            ga.drawLine(pos + uvec2{ 0, 0 }, pos + uvec2{ 0, 4 }, color);
            ga.drawLine(pos + uvec2{ 1, 0 }, pos + uvec2{ 4, 3 }, color);
            ga.drawLine(pos + uvec2{ 3, 3 }, pos + uvec2{ 1, 4 }, color);
            ga.putColor(pos + uvec2{ 3, 5 }, color);
            ga.drawLine(pos + uvec2{ 1, 1 }, pos + uvec2{ 1, 3 }, color);
            ga.drawLine(pos + uvec2{ 2, 2 }, pos + uvec2{ 2, 3 }, color);
        }
        
        { //Pan tool 7x7
            const uvec2 pos { 96, 56 };
            Color color { 0xFF, 0xF1, 0xE8, 0xFF };
            if(mode != PAN) color = { 0xAA, 0xAA, 0xAA, 0xFF };
            ga.putColor(pos + uvec2{ 0, 4 }, color);
            ga.putColor(pos + uvec2{ 1, 5 }, color);
            ga.drawLine(pos + uvec2{ 2, 0 }, pos + uvec2{ 2, 5 }, color);
            ga.drawLine(pos + uvec2{ 3, 3 }, pos + uvec2{ 3, 6 }, color);
            ga.drawLine(pos + uvec2{ 4, 0 }, pos + uvec2{ 4, 6 }, color);
            ga.drawLine(pos + uvec2{ 5, 3 }, pos + uvec2{ 5, 6 }, color);
            ga.drawLine(pos + uvec2{ 6, 1 }, pos + uvec2{ 6, 5 }, color);
        }
        
        { //Save action 7x7
            const uvec2 pos { 104, 56 };
            Color color = { 0xAA, 0xAA, 0xAA, 0xFF };
            if(mode == RUBBER) color = { 0xFF, 0xF1, 0xE8, 0xFF };
            ga.drawLine(pos + uvec2{ 0, 4 }, pos + uvec2{ 4, 0 }, color);
            ga.drawLine(pos + uvec2{ 4, 0 }, pos + uvec2{ 6, 2 }, color);
            ga.drawLine(pos + uvec2{ 6, 2 }, pos + uvec2{ 2, 6 }, color);
            ga.drawLine(pos + uvec2{ 2, 6 }, pos + uvec2{ 0, 4 }, color);
            ga.drawLine(pos + uvec2{ 1, 4 }, pos + uvec2{ 2, 5 }, color);
            ga.drawLine(pos + uvec2{ 2, 3 }, pos + uvec2{ 3, 4 }, color);
            ga.putColor(pos + uvec2{ 2, 4 }, color);
        }
        
        { //Go to select menu 7x7
            const uvec2 pos = { 112, 56 };
            ga.drawLine(pos + uvec2{ 0, 0 }, pos + uvec2{ 6, 6 }, { 0xAA, 0xAA, 0xAA, 0xFF });
            ga.drawLine(pos + uvec2{ 0, 6 }, pos + uvec2{ 6, 0 }, { 0xAA, 0xAA, 0xAA, 0xFF });
        }
        
        if(hasSelection) {
            ga.enableClipInRectangle({ { 0, 0 }, { 128, 52 } });
            ga.drTHICC({
                16.0f*(selection.pos + vec2(mapPosition) + vec2{ selection.size.x >= 0 ? 0 : 1, selection.size.y >= 0 ? 0 : 1 }) - vec2{ selection.size.x >= 0 ? 0 : 1, selection.size.y >= 0 ? 0 : 1 },
                16.0f*selection.size + vec2{ selection.size.x >= 0 ? 0 : 2, selection.size.y >= 0 ? 0 : 2 }
            }, { 0xFF, 0xFF, 0xFF, 0xFF });
            ga.disableClipInReactangle();
        }
        
        if(mouseInsideCanvas) {
            ivec2 pos = mapPosition - ivec2(mouseCanvasPos / 8u);
            ga.print("(" + to_string(-pos.x) + ", " + to_string(-pos.y) + ")", { 74, 67 }, { 0xFF, 0xFF, 0xFF, 0xFF });
        }
        
        if((mode == DRAW || mode == FILL || mode == RUBBER) && mouseInsideCanvas) {
            ivec2 pos = mapPosition - ivec2(mouseCanvasPos / 8u);
            if(pos.x <= 0 && pos.y <= 0) {
                if(mouseCanvasPos.y + 8 >= 56) {
                    ga.drTHICC({ 2u * mouseCanvasPos, { 16, 9 } }, { 255, 255, 255, 255 });
                    ga.fillRectangle({ { mouseCanvasPos.x, 52 }, { 8, 1 } }, { 0x4F, 0x5A, 0x69, 0xFF });
                } else {
                    ga.drTHICC({ 2u * mouseCanvasPos, { 16, 16 } }, { 255, 255, 255, 255 });
                }
            }
        }
        
        if(!notificationMessage.empty()) {
            const auto size = ga.canvasSize();
            const auto textSize = ga.sizeOfText(notificationMessage);
            ga.fillRectangle({ { size.x - textSize.x - 4, 2 }, { textSize.x + 2, textSize.y + 2 } }, { 0xFA, 0x40, 0x5F, 0xFF });
            ga.print(notificationMessage, { size.x - textSize.x - 2, 3 }, { 0xFF, 0xFF, 0xFF, 0xFF });
        }
        
        redraw = false;
    }
    
    void fill(const uvec2 &pos, uint8_t sprite) {
        if(map->at(pos.x, pos.y) == sprite) {
            map->at(pos.x, pos.y) = selectedSprite + 1;
            if(pos.x + 1 < map->getSize().x) fill(pos + uvec2{ 1, 0 }, sprite);
            if(pos.x > 0) fill(pos - uvec2{ 1, 0 }, sprite);
            if(pos.y + 1 < map->getSize().y) fill(pos + uvec2{ 0, 1 }, sprite);
            if(pos.y > 0) fill(pos - uvec2{ 0, 1 }, sprite);
        }
    }
    
    void doMoveMap(const vec2 &d) {
        mapPosAccum += d * 2.0f;
        mapPosAccum.x = glm::min(mapPosAccum.x, 4.0f * 8.0f);
        mapPosAccum.y = glm::min(mapPosAccum.y, 2.0f * 8.0f);
        mapPosAccum.x = glm::max(mapPosAccum.x, (-float(map->getSize().x) + 16 - 4) * 8.0f);
        mapPosAccum.y = glm::max(mapPosAccum.y, (-float(map->getSize().y) + 6 - 2) * 8.0f);
        mapPosition = ivec2(mapPosAccum) / 8;
    }
    
public:
    
    MapEditorScreen(Game &game, const char* name): Level(game, name) {}
    
    void setStuff(Map &map) {
        this->map = &map;
    }
    
};
