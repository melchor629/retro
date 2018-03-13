#pragma once

#include <vector>
#include <UIObject.hpp>
#include <AnimationChain.hpp>

namespace hw {

    using namespace std;
    using namespace retro;

    class Dialog: public UIObject {
        vector<string> paginas;
        size_t pagina = size_t(-1);
        AnimationChain<size_t> aparecer;

        void prepareAnimation();

    protected:
        void setup() override;

        void addPage(std::string &&str);

        void nextPage();

        void prevPage();

        void goToPage(size_t page);

        bool hasNextPage();

        void update(float delta, GameActions &ga) override;

        void draw(GameActions &ga) override;

        void keyUp(int scancode) override;

        void mouseUp(const glm::ivec2 &pos, int button, int clicks) override;

    public:
        Dialog(Game &game, Level &level, const glm::vec2 &pos, const string &name, initializer_list<string> list): UIObject(game, level, pos, name) {
            paginas.assign(list);
        }
    };

}
