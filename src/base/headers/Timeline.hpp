#pragma once

#include <AnimationChain.hpp>
#include <unordered_map>

namespace retro {

    /// Compose a Timeline of animations
    /**
     * Timeline allows you to compose a set of AnimationChain (that it is also
     * a composition of Animation) to make complex animation of different types
     * and properties. The animations in the timeline will run at the same time,
     * but some are able to have some "dependencies" with another animations, 
     * and so, they will run with an extra initial delay. The dependencies kinds
     * are:
     *
     *  - **Nothing**: The animation will start with no delay - add()
     *  - **With another**: The animation will start when another starts - addWith()
     *  - **After another**: The animation will start when another ends - addAfter()
     *
     * The animations are stored in a list and, thus, they have a position associated,
     * position which you can use to reference the other animations. As the
     * animations dependencies are added using an extra initial delay, the position
     * will remain the same during the lifetime of the Timeline object. With this
     * identification, you can even use hardcoded positions, because you know where
     * an animation is added. The first animation is position 0.
     *
     * The AnimationChain object is copied or moved into the list, so the modifications
     * done in the original object won't affect the one in the list. (As you can image
     * of [std::vector](http://en.cppreference.com/w/cpp/container/vector) ).
     *
     * Timeline exposes some of the same methods of AnimationChain and Animation,
     * so in general is safe to use them indistingibly. This methods are animate(),
     * isCompleted(), reset() and getDuration().
     *
     *  > **FYI** to avoid problems for calling methods on a unknown template type
     *  > in runtime, I use [std::bind](http://en.cppreference.com/w/cpp/utility/functional/bind)
     *  > to bind the correct methods with the instance of the object stored in
     *  > the `std::vector`. See Timeline::Anim code :)
     *
     * An example:
     * ```
     *  this->timelineAnimation.add(textAnimation); //The same of AnimationChain example
     *  this->timelineAnimation.addWith(0, AnimationChain<Color>{
     *      Animation<Color>(interpolator::Linear<>(),
     *                       1,
     *                       0xFFFFFF_rgb,
     *                       0xFFFF00_rgb,
     *                       [this] (auto &c) { this->setTextColor(c); }),
     *      Animation<Color>(interpolator::Linear<>(),
     *                       1,
     *                       0xFFFF00_rgb,
     *                       0xFF00FF_rgb,
     *                       [this] (auto &c) { this->setTextColor(c); }),
     *      Animation<Color>(interpolator::Linear<>(),
     *                       1,
     *                       0xFF00FF_rgb,
     *                       0x00FFFF_rgb,
     *                       [this] (auto &c) { this->setTextColor(c); })
     *  });
     * ```
     */
    class Timeline {
        struct Anim {
            void* ptr;
            Animation<float> delay;
            std::function<bool()> isCompleted;
            std::function<void(float)> animate;
            std::function<float()> getDuration;
            std::function<void()> reset;
            template<class T> Anim(AnimationChain<T>* p) {
                ptr = p;
                isCompleted = std::bind(&AnimationChain<T>::isCompleted, p);
                animate = std::bind(&AnimationChain<T>::animate, p, std::placeholders::_1);
                getDuration = std::bind(&AnimationChain<T>::getDuration, p);
                reset = std::bind(&AnimationChain<T>::reset, p);
            }
        };

        std::vector<Anim> chains;
        bool allEnded = false;

    public:

        /// Adds an animation that will apply after the animation[i]
        /**
         * The animation will be applied when the animation with numer `i` has
         * ended. Will be added a delay equal to the delay and duration
         * of the animation `i`. The delay is not added in the AnimationChain object,
         * is set appart. To get duration + delay of this animation chain, use
         * durationWithDelay() with the number returned in this method.
         * @param i The position of the AnimationChain that will follow to the new added
         * @param chain The new AnimationChain to add to the Timeline
         * @return The position of the animation in the list
         * @see Timeline - to know about the positioning of the animations
         **/
        template<typename T>
        size_t addAfter(size_t i, const AnimationChain<T> &chain) {
            chains.push_back(new AnimationChain<T>(chain));
            chains.back().delay = delay<float>([i, this] () { return this->chains[i].getDuration() + this->chains[i].delay.getDuration(); });
            return chains.size() - 1;
        }

        /// Adds an animation that will apply after the animation[i]
        /**
         * The animation will be applied when the animation with numer `i` has
         * ended. Will be added a delay equal to the delay and duration
         * of the animation `i`. The delay is not added in the AnimationChain object,
         * is set appart. To get duration + delay of this animation chain, use
         * durationWithDelay() with the number returned in this method.
         * @param i The position of the AnimationChain that will follow to the new added
         * @param chain The new AnimationChain to add to the Timeline
         * @return The position of the animation in the list
         * @see Timeline - to know about the positioning of the animations
         **/
        template<typename T>
        size_t addAfter(size_t i, AnimationChain<T> &&chain) {
            chains.push_back(new AnimationChain<T>(std::forward<AnimationChain<T>>(chain)));
            chains.back().delay = delay<float>([i, this] () { return this->chains[i].getDuration() + this->chains[i].delay.getDuration(); });
            return chains.size() - 1;
        }

        /// Adds an animation that will apply at the same time of animation[i] does
        /**
         * The animation will be applied when the animation with number `i` starts
         * appling. Will be added a delay equal to the delay of the animation `i`.
         * The delay is not added in the AnimationChain object, is set appart. To
         * get duration + delay of this animation chain, use durationWithDelay() with
         * the number returned in this method.
         * @param i The position of the AnimationChain that will run along this one
         * @param chain The new AnimationChain to add to the Timeline
         * @return The position of the animation in the list
         * @see Timeline - to know about the positioning of the animations
         */
        template<typename T>
        size_t addWith(size_t i, const AnimationChain<T> &chain) {
            chains.push_back(new AnimationChain<T>(chain));
            chains.back().delay = delay<float>([i, this] () { return this->chains[i].delay.getDuration(); });
            return chains.size() - 1;
        }

        /// Adds an animation that will apply at the same time of animation[i] does
        /**
         * The animation will be applied when the animation with number `i` starts
         * appling. Will be added a delay equal to the delay of the animation `i`.
         * The delay is not added in the AnimationChain object, is set appart. To
         * get duration + delay of this animation chain, use durationWithDelay() with
         * the number returned in this method.
         * @param i The position of the AnimationChain that will run along this one
         * @param chain The new AnimationChain to add to the Timeline
         * @return The position of the animation in the list
         * @see Timeline - to know about the positioning of the animations
         */
        template<typename T>
        size_t addWith(size_t i, AnimationChain<T> &&chain) {
            chains.push_back(new AnimationChain<T>(std::forward<AnimationChain<T>>(chain)));
            chains.back().delay = delay<float>([i, this] () { return this->chains[i].delay.getDuration(); });
            return chains.size() - 1;
        }

        /// Adds an animation that will apply from the beginning of the timeline
        /**
         * The animation has no beginning extra delay, as opposite to addWith() and
         * addAfter(). The durationWithDelay() and AnimationChain::getDuration() will
         * return the same time.
         * @param chain The new AnimationChain to add to the Timeline
         * @return The position of the animation in the list
         * @see Timeline - to know about the positioning of the animations
         */
        template<typename T>
        size_t add(const AnimationChain<T> &chain) {
            chains.push_back(new AnimationChain<T>(chain));
            chains.back().delay = delay<float>(0);
            return chains.size() - 1;
        }

        /// Adds an animation that will apply from the beginning of the timeline
        /**
         * The animation has no beginning extra delay, as opposite to addWith() and
         * addAfter(). The durationWithDelay() and AnimationChain::getDuration() will
         * return the same time.
         * @param chain The new AnimationChain to add to the Timeline
         * @return The position of the animation in the list
         * @see Timeline - to know about the positioning of the animations
         */
        template<typename T>
        size_t add(AnimationChain<T> &&chain) {
            chains.push_back(new AnimationChain<T>(std::forward<AnimationChain<T>>(chain)));
            chains.back().delay = delay<float>(0);
            return chains.size() - 1;
        }

        /// Gets an AnimationChain positioned at `pos` (safe version)
        /**
         * This method checks if the position is valid and the type of the template
         * argument of AnimationChain corresponds to the type in that position.
         * If the position is invalid or the type mismatch, an exception will be
         * thrown. Is the safe version of at().
         *
         *  > **Note**: If the type mismatches, the excepction in Windows (using
         *  > MSVC compiler) is descriptive enough. In other SO and compilers,
         *  > the types (annotated between `'`) are a weird text that can be
         *  > converted (somehow) to a human readable version using `c++filt`.
         * @param pos The position of the animation to obtain
         * @return A reference to the AnimationChain object stored at this position
         */
        template<typename T>
        AnimationChain<T>& get(size_t pos) {
            auto a = reinterpret_cast<AnimationChain<float>*>(chains.at(pos).ptr);
            if(a->itype == std::type_index(typeid(T))) {
                return *reinterpret_cast<AnimationChain<T>*>(a);
            } else {
                throw std::runtime_error("Type in AnimationChain mismatch: expected '" +
                    std::string(typeid(T).name()) + "', got '" + a->itype.name() + "' instead");
            }
        }

        /// Gets an AnimationChain positioned at `pos` (unsafe version)
        /**
         * This method supposes what are you doing. So no checks are done. If
         * the position is invalid or the template argument request is not correct,
         * undefined behaviour will occur, and even a crash.
         * @param pos The position of the animation to obtain
         * @return A reference to the AnimationChain object stored at this position
         */
        template<typename T>
        AnimationChain<T> at(size_t pos) {
            return *reinterpret_cast<AnimationChain<T>*>(chains[pos].ptr);
        }

        /// Gets the duration of an animation plus the delay introduced by the dependencies
        float durationWithDelay(size_t pos) {
            return chains[pos].getDuration() + chains[pos].delay.getDuration();
        }

        /// Applies one animation step
        void animate(float delta) {
            if(!isCompleted()) {
                allEnded = true;
                for(auto &anim: chains) {
                    if(!anim.delay.isCompleted()) {
                        allEnded = false;
                        anim.delay.animate(delta);
                    } else if(!anim.isCompleted()) {
                        allEnded = false;
                        anim.animate(delta);
                    }
                }
            }
        }

        /// Resets the animation to the initial state
        void reset() {
            for(auto &anim: chains) anim.reset();
        }

        /// Returns `true` if all the animations have ended
        constexpr bool isCompleted() const {
            return allEnded;
        }

        /// Returns the duration of the animation
        float getDuration() const {
            float dur = 0;
            for(auto &anim: chains) dur = std::max(dur, anim.getDuration() + anim.delay.getDuration());
            return dur;
        }
    };

}
