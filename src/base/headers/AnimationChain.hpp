#pragma once

#include <Animation.hpp>
#include <vector>
#include <typeinfo>
#include <typeindex>

namespace retro {

    /// Chain animations of the same property to make large animations with ease
    /**
     * STL vector-like class that allows you to chain animations to make more complex
     * animations without pain. Use Animation and delay() to make simple animations
     * and add some delay between them and use the AnimationChain the same way as an
     * Animation object (you have animation() and isCompleted() methods too).
     *
     * When the animation has started, the structure shall not be modified. If modified,
     * the behaviour is undefined.

     * The Animation object is copied or moved into the list, so the modifications
     * done in the original object won't affect the one in the list (same behaviour
     * of [std::vector](http://en.cppreference.com/w/cpp/container/vector) ).
     *
     *  > **Recomendation**: to avoid duplication of the Animation::Setter function,
     *  > put it in a lambda function and reuse it as many times as you need.
     *
     * Let's show an example. That animation waits 2s and then, every 0.1s shows a new
     * character of the text on the screen.
     *
     * ```
     * //Imagine that we have an UIObject with a fullText string that holds the final text
     * textAnimation = {
     *     delay<size_t>(2), //Start after 2s
     *     Animation<size_t>(interpolator::Linear<>(),
     *                       utf8::distance(fullText.begin(), fullText.end()) * 0.1,
     *                       0,
     *                       utf8::distance(fullText.begin(), fullText.end()),
     *                       [this] (auto &p) {
     *                           //Correct way to show one character
     *                           auto it = fullText.begin();
     *                           utf8::advance(it, p, fullText.end());
     *                           utf8::next(it, fullText.end());
     *                           this->setText(fullText.substr(0, it - fullText.begin()));
     *                       })
     * };
     * ```
     * @see http://en.cppreference.com/w/cpp/container/vector std::vector
     * @see delay()
     */
    template<typename T>
    class AnimationChain {
        using anim_list = std::vector<Animation<T>>;
        anim_list animations;
        typename anim_list::iterator it;
        bool hasStarted = false;

    public:

        typedef Animation<T> value_type;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef typename anim_list::iterator iterator;
        typedef typename anim_list::const_iterator const_iterator;
        typedef typename anim_list::reverse_iterator reverse_iterator;
        typedef typename anim_list::const_reverse_iterator const_reverse_iterator;
        typedef typename anim_list::difference_type difference_type;
        typedef typename anim_list::size_type size_type;

        /// Type info about the T template argument
        const std::type_info &type = typeid(T);
        /// Type index for the type info
        const std::type_index itype = type;

        constexpr AnimationChain() {}

        constexpr AnimationChain(const Animation<T> &o) { animations.push_back(o); }
        constexpr AnimationChain(Animation<T> &&o) { animations.push_back(std::forward<value_type>(o)); }

        constexpr AnimationChain(const AnimationChain<T> &o) {
            *this = o;
        }

        constexpr AnimationChain(AnimationChain<T> &&o) {
            *this = std::forward<AnimationChain<T>>(o);
        }

        constexpr AnimationChain(std::initializer_list<Animation<T>> init): animations(init) {}

        constexpr AnimationChain<T>& operator=(const AnimationChain<T> &o) {
            animations = o.animations;
            return *this;
        }

        constexpr AnimationChain<T>& operator=(AnimationChain<T> &&o) {
            animations = std::move(o.animations);
            return *this;
        }

        constexpr AnimationChain<T>& operator=(std::initializer_list<Animation<T>> init) {
            animations = init;
            return *this;
        }

        constexpr void assign(size_type count, const value_type& value) {
            animations.assign(count, value);
        }

        template<class InputIt>
        constexpr void assign(InputIt first, InputIt last) {
            animations.assign(first, last);
        }

        constexpr void assign(std::initializer_list<value_type> ilist) {
            animations.assign(ilist);
        }

        constexpr reference at(size_type pos) {
            return animations.at(pos);
        }

        constexpr const_reference at(size_type pos) const {
            return animations.at(pos);
        }

        constexpr reference operator[](size_type pos) { return animations[pos]; }
        constexpr const_reference operator[](size_type pos) const { return animations[pos]; }

        constexpr iterator begin() noexcept { return animations.begin(); }
        constexpr const_iterator begin() const noexcept { return animations.begin(); }
        constexpr const_iterator cbegin() const noexcept { return animations.cbegin(); }
        constexpr iterator end() noexcept { return animations.end(); }
        constexpr const_iterator end() const noexcept { return animations.end(); }
        constexpr const_iterator cend() const noexcept { return animations.cend(); }
        constexpr reverse_iterator rbegin() noexcept { return animations.rbegin(); }
        constexpr const_reverse_iterator rbegin() const noexcept { return animations.rbegin(); }
        constexpr const_reverse_iterator crbegin() const noexcept { return animations.crbegin(); }
        constexpr reverse_iterator rend() noexcept { return animations.rend(); }
        constexpr const_reverse_iterator rend() const noexcept { return animations.rend(); }
        constexpr const_reverse_iterator crend() const noexcept { return animations.crend(); }

        constexpr bool empty() const noexcept { return animations.empty(); }
        constexpr size_type size() const noexcept { return animations.size(); }
        constexpr size_type max_size() const noexcept { return animations.max_size(); }

        constexpr void clear() noexcept { hasStarted = false; return animations.clear(); }
        constexpr iterator insert(const_iterator pos, const_reference value) { return animations.insert(pos, value); }
        constexpr iterator insert(const_iterator pos, value_type &&value) { return animations.insert(pos, std::forward<value_type>(value)); }
        constexpr iterator insert(const_iterator pos, size_type count, const_reference value) { return animations.insert(pos, count, value); }
        template<class InputIt>
        constexpr iterator insert(const_iterator pos, InputIt first, InputIt last) { return animations.insert(pos, first, last); }
        constexpr iterator insert(const_iterator pos, std::initializer_list<value_type> ilist) { return animations.insert(pos, ilist); }
        template<class... Args>
        constexpr iterator emplace(const_iterator pos, Args&&... args) { return animations.emplace(pos, args...); }
        constexpr iterator erase(const_iterator pos) { return animations.erase(pos); }
        constexpr iterator erase(const_iterator first, const_iterator last) { return animations.erase(first, last); }
        constexpr void push_back(const value_type& value) { animations.push_back(value); }
        constexpr void push_back(value_type &&value) { animations.push_back(std::forward<value_type>(value)); }
        template<class... Args>
        constexpr void emplace_back(const_iterator pos, Args&&... args) { return animations.emplace(pos, args...); }
        constexpr void pop_back() { animations.pop_back(); }

        constexpr void swap(AnimationChain<T> &other) { animations.swap(other.animations); }

        /// Inserts using a numeric position instead of an iterator
        constexpr iterator insert(size_type pos, const_reference value) { return insert(begin() + pos, value); }
        /// Inserts using a numeric position instead of an iterator
        constexpr iterator insert(size_type pos, value_type &&value) { return insert(begin() + pos, std::forward<value_type>(value)); }
        /// Inserts using a numeric position instead of an iterator
        constexpr iterator insert(size_type pos, std::initializer_list<value_type> ilist) { return insert(begin() + pos, ilist); }
        /// Delete an item using a numeric position
        constexpr iterator erase(size_type pos) { return erase(begin() + pos); }
        /// Delete a range of items using a numeric positions
        constexpr iterator erase(size_type first, size_type last) { return animations.erase(begin() + first, begin() + last); }


        /// Applies one animation step
        void animate(float delta) {
            if(!hasStarted) {
                hasStarted = true;
                it = animations.begin();
            }

            if(!isCompleted()) {
                it->animate(delta);
                if(it->isCompleted()) {
                    it++;
                }
            }
        }

        /// Changes the progress of the animation chain to completed
        void complete() {
            while(!isCompleted()) {
                it->complete();
                it++;
            }
        }

        /// Changes the progress of the animation chain to that _(absoulte)_ value (in seconds)
        void setProgress(float time) {
            reset();
            hasStarted = true;
            it = animations.begin();
            while(time > 0) {
                it->setProgress(time);
                time -= it->getDuration();
            }
        }

        /// Resets the animation to the initial state
        void reset() {
            hasStarted = false;
            for(auto &anim: animations) anim.reset();
        }

        /// Returns `true` if all the animations have ended
        constexpr bool isCompleted() const {
            return hasStarted && it == animations.end();
        }

        /// Returns the duration of the animation
        float getDuration() const {
            float dur = 0;
            for(auto &anim: animations) dur += anim.getDuration();
            return dur;
        }

    };

    /// Empty animation that is useful for making delays in AnimationChain
    /**
     * Adds a delay to the AnimationChain, with an Animation that does
     * nothing but not letting the AnimationChain to change to the next
     * animation until this bougs Animation ends.
     * @param duration The duration of the delay in seconds
     * @return An Animation instance that does nothing
     */
    template<typename T>
    inline auto delay(float duration) -> Animation<T> {
        return Animation<T>(interpolator::Linear<>(), duration, 0, 1, [] (auto&) {});
    }

    /// Empty animation that is useful for making delays in AnimationChain
    /**
     * Adds a delay to the AnimationChain, with an Animation that does
     * nothing but not letting the AnimationChain to change to the next
     * animation until this bougs Animation ends. In this overload, the
     * duration is calculated from a function.
     * @param duration The duration of the delay in seconds, calculated from the function
     * @return An Animation instance that does nothing
     */
    template<typename T>
    inline auto delay(typename Animation<T>::Duration duration) -> Animation<T> {
        return Animation<T>(interpolator::Linear<>(), duration, T(), T(), [] (auto&) {});
    }

    template<typename T>
    auto operator==(const AnimationChain<T> &a, const AnimationChain<T> &b) -> bool {
        if(a.size() == b.size()) {
            bool equals = true;
            auto ita = a.begin();
            auto itb = b.begin();
            while(equals && ita != a.end()) {
                equals = *ita == *itb;
            }
            return equals;
        } else {
            return false;
        }
    }

    template<typename T>
    auto operator!=(const AnimationChain<T> &a, const AnimationChain<T> &b) -> bool {
        return !(a == b);
    }

}

namespace std {
    template<class T>
    void swap(retro::AnimationChain<T>& lhs, retro::AnimationChain<T>& rhs) {
        lhs.swap(rhs);
    }
}
