
#include <Player.hpp>
#include <MapObject.hpp>
#include <GameActions.hpp>
#include <Animation.hpp>
#include <AnimationChain.hpp>
#include <Timeline.hpp>
#include <Image.hpp>
#include <utf8.h>
#include <CollisionDetection.hpp>

class UnTextoCualquiera: public UIObject {
    Timeline textAnimation;
    string fullText = "Un texto normal y cualquiera\nSi\txD";
public:
    virtual void setup() override {
        frame.pos = { 500, 40 };
        setFont("Ubuntu-R.ttf", 26);
        setTextBoxLimit(BoxLimit::FixedWidth, { 300, 300 });
        setAlign(TextVerticalAlign::Center);
        setAlign(TextHorizontalAlign::Center);

        UILabel otro(this, { 10, 10 }, "otroTexto");
        otro.setFont("Ubuntu-R.ttf", 11);
        otro.setText("Otro texto normal y corriente");
        addSubobject(std::move(otro));

        AnimationChain<size_t> textAnimation = {
            delay<size_t>(2),
            Animation<size_t>(interpolator::Linear<>(),
                              utf8::distance(fullText.begin(), fullText.end()) * 0.1,
                              0,
                              utf8::distance(fullText.begin(), fullText.end()),
                              [this] (auto &p) {
                                  auto it = fullText.begin();
                                  utf8::advance(it, p, fullText.end());
                                  utf8::next(it, fullText.end());
                                  this->setText(fullText.substr(0, it - fullText.begin()));
                              })
        };

        AnimationChain<vec2> textPosition = {
            Animation<vec2>(interpolator::CubicInOut<>(),
                            3,
                            frame.pos,
                            { 800, 100 },
                            [this] (auto &p) { frame.pos = p; })
        };

        this->textAnimation.add(textAnimation);
        this->textAnimation.addAfter(0, textPosition);
        this->textAnimation.addWith(1, AnimationChain<Color>{
            Animation<Color>(interpolator::Linear<>(),
                             1,
                             0xFFFFFF_rgb,
                             0xFFFF00_rgb,
                             [this] (auto &c) { this->setTextColor(c); }),
            Animation<Color>(interpolator::Linear<>(),
                             1,
                             0xFFFF00_rgb,
                             0xFF00FF_rgb,
                             [this] (auto &c) { this->setTextColor(c); }),
            Animation<Color>(interpolator::Linear<>(),
                             1,
                             0xFF00FF_rgb,
                             0x00FFFF_rgb,
                             [this] (auto &c) { this->setTextColor(c); })
        });
    }

    virtual void update(float delta, GameActions &ga) override {
        UIObject::update(delta, ga);
        if(!textAnimation.isCompleted()) textAnimation.animate(delta);
    }

    virtual void draw(GameActions &ga) override {
        ga.fillRectangle({ { 0, 0 }, getTextSize() }, 0x22222277_rgba);
        renderText(ga);
        UIObject::draw(ga);
    }

    UnTextoCualquiera(Game &game, Level &level, const vec2 &pos, const string &name): UIObject(game, level, pos, name) {}
};

class Coquein: public ControlledPlayer {

    vector<Sprite> verticalAnimation;
    vector<Sprite> horizontalAnimation;
    vector<Sprite> diagonal1Animation;
    vector<Sprite> diagonal2Animation;
    int dir = 0;

    int i = 0;
    float timeAccum = 0.0f;

    inline vector<Sprite> &currentAnimation() {
        if(abs(instantSpeed.x) < 0.001f && abs(instantSpeed.y) < 0.001f) {
            if(dir == 1) return verticalAnimation;
            if(dir == 2) return horizontalAnimation;
            if(dir == 3) return diagonal1Animation;
            if(dir == 4) return diagonal2Animation;
        } else {
            if(abs(instantSpeed.y) > 0.001f && abs(instantSpeed.x) <= 0.001f) { dir = 1; return verticalAnimation; }
            if(abs(instantSpeed.x) > 0.001f && abs(instantSpeed.y) <= 0.001f) { dir = 2; return horizontalAnimation; }
            if((instantSpeed.x > 0.0f && instantSpeed.y > 0.0f) || (instantSpeed.x < 0.0f && instantSpeed.y < 0.0f)) { dir = 3; return diagonal1Animation; }
            if((instantSpeed.x > 0.0f && instantSpeed.y < 0.0f) || (instantSpeed.x < 0.0f && instantSpeed.y > 0.0f)) { dir = 4; return diagonal2Animation; }
        }
        return verticalAnimation;
    }

public:

    Coquein(Game &game, Level &level, const glm::vec2 &pos, const char* name): ControlledPlayer(game, level, pos, name) {}

    virtual void setup() override {
        playerSpeed = 10.0;
    }

    virtual void update(float d, GameActions &ga) override {
        ControlledPlayer::update(d, ga);
        if(abs(instantSpeed.x) < 0.09f && abs(instantSpeed.y) < 0.09f) {
            timeAccum = 0.0f;
            i = 0;
        } else {
            timeAccum += d;
            float timeForSpeed = (playerSpeed * 1.5f - glm::length(instantSpeed)) * d;
            //Avoid strange things when teleporting
            if(timeForSpeed > 0 && timeAccum >= timeForSpeed) {
                i = (i + 1) % currentAnimation().size();
                timeAccum = timeAccum - timeForSpeed;
            }
        }
    }

    virtual void draw(GameActions&) override {
        Sprite spr = currentAnimation()[i];
        //ga.drawRectangle(frame, { 0xFF, 0xFF, 0xFF, 0xAB });
        spr.draw(frame.pos - vec2{ 3, 3 });
    }

    void setSprites(Sprites &sprites) {
        frame.size = { 3, 3 };

        verticalAnimation.push_back(sprites[7]);
        verticalAnimation.push_back(sprites[8]);
        verticalAnimation.push_back(sprites[9]);
        verticalAnimation.push_back(sprites[8]);
        verticalAnimation.push_back(sprites[7]);
        verticalAnimation.push_back(sprites[6]);
        verticalAnimation.push_back(sprites[5]);
        verticalAnimation.push_back(sprites[6]);

        horizontalAnimation.push_back(sprites[23]);
        horizontalAnimation.push_back(sprites[24]);
        horizontalAnimation.push_back(sprites[25]);
        horizontalAnimation.push_back(sprites[24]);
        horizontalAnimation.push_back(sprites[23]);
        horizontalAnimation.push_back(sprites[22]);
        horizontalAnimation.push_back(sprites[21]);
        horizontalAnimation.push_back(sprites[22]);

        diagonal1Animation.push_back(sprites[39]);
        diagonal1Animation.push_back(sprites[40]);
        diagonal1Animation.push_back(sprites[41]);
        diagonal1Animation.push_back(sprites[40]);
        diagonal1Animation.push_back(sprites[39]);
        diagonal1Animation.push_back(sprites[38]);
        diagonal1Animation.push_back(sprites[37]);
        diagonal1Animation.push_back(sprites[38]);

        diagonal2Animation.push_back(sprites[55]);
        diagonal2Animation.push_back(sprites[56]);
        diagonal2Animation.push_back(sprites[57]);
        diagonal2Animation.push_back(sprites[56]);
        diagonal2Animation.push_back(sprites[55]);
        diagonal2Animation.push_back(sprites[54]);
        diagonal2Animation.push_back(sprites[53]);
        diagonal2Animation.push_back(sprites[54]);
    }

};

class Obstaculo: public Object, public Collisionable {
  
    Optional<Sprite> sprite;
    
public:
    
    Obstaculo(Game &game, Level &level, const glm::vec2 &pos, const char* name): Object(game, level, pos, name) {}
    
    virtual Frame& getFrame() override { return frame; }
    virtual const Frame& getFrame() const override { return frame; }
    
    virtual void setup() override {
        frame.size = { 8, 8 };
    }
    
    virtual void update(float, GameActions&) override {}
    
    virtual void draw(GameActions&) override {
        if(sprite) sprite->draw(Frame{ frame.pos, frame.size / 8.0f });
    }
    
    void setSprite(Sprite sprite) {
        this->sprite = sprite;
        frame.size = sprite.frame().size;
    }
    
};


class GameTestLevel: public Level {
    int i = 0;
    Sprites sprites;
    Animation<vec2> cameraAnimation;
    Image* image;

protected:
    virtual void setup() override {
        i = 0;
        sprites.load("sitt.spr");
        sprites.regenerateTextures();
        addObject<MapObject>({ 0, 0 }, "sitt.map");
        addObject<Coquein>({ 64, 64 }, "player");
        addObject<Obstaculo>({ 15, 15 }, "obstaculo");
        getObjectByName<Coquein>("player")->setSprites(sprites);
        getObjectByPosition<Obstaculo>({ 16, 16 })->setSprite(sprites[4]);
        addObject<UnTextoCualquiera>({ 0, 0 }, "untexto");
        image = new Image("Space-Desktop.jpg", game());
        image->generateAndDestroy();
    }

    virtual void cleanup() override {
        delete image;
    }

    virtual void update(float delta) override {
        i++;
        Coquein& player = *getObjectByName<Coquein>("player");
        MapObject& map = *getObjectByName<MapObject>("sitt");
        const auto position = [&map, this] (const vec2 &pos) -> vec2 {
            return max(vec2{ 0, 0 }, min(pos - vec2(ga.canvasSize() / 2u), map.getFrame().size));
        };
        if(cameraAnimation.isCompleted()) {
            ga.camera(position(player.getFrame().pos));
        } else if(!cameraAnimation.isCompleted()) {
            cameraAnimation.animate(delta);
            cameraAnimation.updateFinalValue(player.getFrame().pos);
        }

        uint8_t m = map.at(player.getFrame().pos / 8.0f);
        if(m == 2 || m == 3 || m == 4 || m == 5) {
            cameraAnimation = Animation<vec2>(interpolator::CubicInOut<>(), .5, player.getFrame().pos, vec2{ 64, 64 }, [this, position] (const vec2 &v) {
                ga.camera(position(v));
            });
            player.getFrame().pos = { 64, 64 };
        }
    }

    virtual void keyUp(int scancode) override {
        if(ga.isModKeyPressed(KMOD_GUI) && scancode == SDL_SCANCODE_S) {
            game().saveGame("prueba");
        }
        if(ga.isModKeyPressed(KMOD_GUI) && scancode == SDL_SCANCODE_R) {
            game().restoreGame("prueba");
        }
    }

    virtual bool predraw() override {
        //ga.clear(0);
        uvec2 size = ga.canvasSize();
        vec2 scale = { float(image->getSize().x) / float(size.x), float(image->getSize().y) / float(size.y) };
        auto imgSize = vec2(size) * scale / 2.f;
        image->drawSection(Frame{ ga.camera() * (vec2(image->getSize()) - imgSize) / vec2(350, 220), imgSize }, Frame{ ga.camera(), size });
        return i == 1 || true;
    }

    virtual void draw() override {
        uvec2 size = ga.canvasSize();
        ga.drawLine({ size.x, 0 }, { 0, size.y }, 6);
        ga.fillRectangle({ { 10, 10 }, { 10, 10 } }, 72 + ((i / 30) % 8));
        ga.print(to_string(72 + ((i / 30) % 8)), { 1, 1 }, 7);
        ga.drawRectangle({ { 10, 10 }, { 10, 10 } }, 4);
        ga.putColor({ 100, 0 }, 38);
        ga.putColor({ 100, 1 }, 39);
        ga.putColor({ 100, 70 }, 36);
        ga.putColor({ 100, 71 }, 37);
        ga.fillCircle({ size.x/2, size.y/2 }, 10, 43);
        ga.drawCircle({ size.x/2, size.y/2 }, 10, 20);
        ga.putColor({ size.x/2, size.y/2 }, 7);
        ga.print(to_string(ga.camera().x) + "," + to_string(ga.camera().y), ga.camera());
        ga.print(to_string(getObjectByName<Coquein>("player")->getFrame().pos.x) + "," + to_string(getObjectByName<Coquein>("player")->getFrame().pos.y), ga.camera() + vec2(0, 8));
    }

public:
    GameTestLevel(Game &game, const char* name): Level(game, name), sprites(game) {}
};

class GameTest: public Game {

protected:
    virtual void setup() override {
        importPalette("palette.aco");
        loadFont("8b.ttf");
        addLevel<GameTestLevel>("level", true);
        //audio.loadMusic("onestop.midi"); audio.playMusic("onestop");
        //audio.loadMusic("sc.flac"); audio.playMusic("sc");
    }

    virtual void cleanup() override {}

public:
    GameTest(const Builder &builder): Game(builder) {}
};

int main(int argc, char* argv[]) {
    set_terminate([] () {
        Logger &log = Logger::getLogger("Exception Handler");
        log.error("Uncaught exception");
        try {
            rethrow_exception(current_exception());
        } catch(const exception &e) {
            log.error("%s", e.what());
        }

        exit(9);
    });

    if(argc > 1 && !strcmp("--editor", argv[1])) {
        auto g = windowedBuilder<Editor>(1280, 720);
        g->setFont("8b.ttf");
        g->setPalette("palette.gpl");
        g->loop();
        delete g;
    } else {
#if defined(__ANDROID__) || defined(__IOS__)
        auto g = mobileBuilder<GameTest>();
#else
        auto g = windowedBuilder<GameTest>(1280, 720);
#endif
        g->loop();
        delete g;
    }
    return 0;
}

#define GAME_INCLUDED 1
