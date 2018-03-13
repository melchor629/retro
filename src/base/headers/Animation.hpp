#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <memory>

namespace retro {

    /// Interpolator for animations
    /**
     * An interpolator modifies the animation to have a different
     * effect than a linear one. Use one implementation or implement one
     * yourself to make animations look different.
     *
     * The animation is done with the following equation:
     * `from + (to - from) * InterpolatorValue`
     *
     * Is a rect equation, yep, but the interpolator makes the animation
     * look different. Use them wisely.
     *
     * To get a visual reference go to http://easings.net/
     **/
    template<class T = float>
    struct Interpolator {
        /**
         * Calculates the value for this interpolation given a percentage
         * of animation done (between 0 and 1).
         *
         * **Note to implementators**: I recommend you to make your function to be a `constexpr` function
         * @param perc Percentage done of the animation [0..1]
         * @return Value for the equation `from + (to - from) * InterpolatorValue`
         **/
        virtual /* constexpr */ T interpolate(T perc) const = 0;
        virtual ~Interpolator() {}
    };

    ///Animates properties of objects or other kind of ethereum entities
    /**
     * The type of `Arg` must be a numeric one or any kind of type that
     * defines the following operators:
     *
     *  - `Arg operator+(const Arg &a, const Arg &b)` Addition of the same type
     *  - `Arg operator-(const Arg &a, const Arg &b)` Substraction of the same type
     *  - `Arg operator*(const Arg &a, float b)` Scalar multiplication
     *
     * To make an animation, you need:
     *
     *  - A property to animate
     *  - An initial value
     *  - A final value
     *  - A setter of the property
     *  - A duration
     *  - An interpolator (see Interpolator)
     *
     * After, you need to create an instance of Animation, f.e.:
     * ```
     * using namespace retro;
     * using namespace glm;
     * auto anim = Animation<vec2>(interpolator::Linear<>(), 1, vec2{ 64, 64 }, vec2{ 128, 0 }, [this] (auto &v) { setPosition(v); });
     * ...
     * anim.animate(delta); //This in an update method, delta comes from these methods
     * ```
     *
     * Animates during 1s, from (64, 64) to (128, 0) the position of something.
     *
     * You can always check if the animation is completed or not with
     * isCompleted(), or update the duration and/or the final value of
     * the animation with updateAnimation().
     *
     * If the property type of something you're trying to animate does weird things,
     * in general due to the use of integer types, you can specialize Animation::animate
     * to fix that. Here's an example for Color:
     *
     * ```
     *
     * const float perc = animDone / duration();
     *     if(animDone < 0.0) {
     *     if(setter) setter(from); //Should not happen this case :/
     *         animDone += delta;
     *     } else if(animDone < duration()) {
     *         const float i = interpolator->interpolate(perc);
     *         const float r = float(from.r) + (float(to.r) - float(from.r)) * i;
     *         const float g = float(from.g) + (float(to.g) - float(from.g)) * i;
     *         const float b = float(from.b) + (float(to.b) - float(from.b)) * i;
     *         const float a = float(from.a) + (float(to.a) - float(from.a)) * i;
     *         if(setter) setter({ r, g, b, a });
     *         animDone += delta;
     *     } else {
     *         if(setter) setter(to); //Protect from large animations
     *     }
     * }
     *
     * ```
     **/
    template<typename Arg>
    struct Animation {
        typedef std::function<void(const Arg&)> Setter; ///< The signature of setter functions
        typedef std::function<float()> Duration; ///< Duration from a function, should not be modified when the animation is running

        std::shared_ptr<Interpolator<float>> interpolator = nullptr; ///< The inmutable interpolation implementation
        Setter setter; ///< The setter (lambda) function
        Arg from, to;
        Duration duration = [] () { return 0.0f; };
        float animDone = 0.0f;

        /**
         * Creates a new initialized animation with the following parameters.
         * @param interp An interpolator to be used in the animation
         * @param duration Duration of the animation in seconds
         * @param from The initial value of the animation
         * @param to The final value of the animation
         * @param setter A function that changes the property when a new value is available
         **/
        template<template<typename> class Inter>
        Animation(Inter<float> interp, float duration, Arg from, Arg to, const Setter &setter):
        interpolator(new Inter<float>(interp)),
        setter(setter),
        from(from),
        to(to),
        duration([duration] () -> float { return duration; }) {
            static_assert(std::is_base_of<Interpolator<float>, Inter<float>>::value, "Type of interpolator must inherit from Interpolator");
        }

        /**
         * Creates a new initialized animation with the following parameters.
         * @param interp An interpolator to be used in the animation
         * @param duration Duration of the animation in seconds, calculated from a function
         * @param from The initial value of the animation
         * @param to The final value of the animation
         * @param setter A function that changes the property when a new value is available
         **/
        template<template<typename> class Inter>
        Animation(Inter<float> interp, Duration duration, Arg from, Arg to, const Setter &setter):
        interpolator(new Inter<float>(interp)),
        setter(setter),
        from(from),
        to(to),
        duration(duration) {
            static_assert(std::is_base_of<Interpolator<float>, Inter<float>>::value, "Type of interpolator must inherit from Interpolator");
        }

        /**
         * Creates an unitialized animation. To initialize it, associate a new
         * initialized Animation with the assign operator (`=`).
         */
        Animation() {}

        /// Copy constructor
        Animation(const Animation<Arg> &o) {
            *this = o;
        }

        /// Move constructor
        Animation(Animation<Arg> &&o) {
            *this = std::forward<Animation<Arg>>(o);
        }

        ~Animation() {}

        /**
         * Updates the animation with a new final value. Doesn't alter the duration
         * nor the current time, a jump in the animation can occur.
         * @param to The new final value
         **/
        void updateFinalValue(Arg to) {
            this->to = to;
        }

        /**
         * Updates the animation duration. A jump on the animation
         * can occur.
         **/
        void updateAnimation(float duration) {
            this->duration = duration;
        }

        /**
         * Updates the animation with a new final value and the duration.
         * A jump in the animation can occur.
         * @param to The new final value
         * @param duration The new duration of the animation
         **/
        void updateAnimation(Arg to, float duration) {
            this->to = to;
            this->duration = duration;
        }

        /**
         * Applies the corresponding animation based on the time passed
         * (`delta`). You should call this method inside an update method.
         * @param delta Time passed since last animate call
         **/
        void animate(float delta) {
            const float perc = animDone / duration();
            if(animDone < 0.0) {
                if(setter) setter(from); //Should not happen this case :/
                animDone += delta;
            } else if(animDone < duration()) {
                if(setter) setter(Arg(from + (to - from) * interpolator->interpolate(perc)));
                animDone += delta;
            } else {
                if(setter) setter(to); //Protect from large animations
            }
        }

        /**
         * @return `true` if the animation is completed or unitialized
         **/
        inline bool isCompleted() const { return animDone >= duration(); }

        /// Changes the current state of the animation to complete
        void complete() {
            animDone = duration();
            if(setter) setter(to);
        }

        /// Changes the current progress of the animation to the _(absolute)_ value of the time (in seconds)
        void setProgress(float time) {
            bool wasDone = isCompleted();
            animDone = std::abs(time);
            if(!wasDone && isCompleted()) {
                if(setter) setter(to);
            }
        }

        /// Resets the animation, you can start again the animation.
        inline void reset() { animDone = 0.0; }

        constexpr float getDuration() const {
            return duration();
        }

        /// Asign operator
        inline Animation<Arg>& operator=(const Animation<Arg> &o) {
            interpolator = o.interpolator;
            setter = o.setter;
            from = o.from;
            to = o.to;
            duration = o.duration;
            animDone = o.animDone;
            return *this;
        }

        /// Move-asign operator
        inline Animation<Arg>& operator=(Animation<Arg> &&o) {
            interpolator = o.interpolator; o.interpolator = nullptr;
            setter = std::move(o.setter);
            from = std::move(o.from);
            to = std::move(o.to);
            duration = o.duration;
            animDone = o.animDone;
            return *this;
        }

    };

    ///Specialization of animate for Colors
    template<>
    inline void Animation<Color>::animate(float delta) {
        const float perc = animDone / duration();
        if(animDone < 0.0) {
            if(setter) setter(from); //Should not happen this case :/
            animDone += delta;
        } else if(animDone < duration()) {
            const float i = interpolator->interpolate(perc);
            const float r = float(from.r) + (float(to.r) - float(from.r)) * i;
            const float g = float(from.g) + (float(to.g) - float(from.g)) * i;
            const float b = float(from.b) + (float(to.b) - float(from.b)) * i;
            const float a = float(from.a) + (float(to.a) - float(from.a)) * i;
            if(setter) setter({ r, g, b, a });
            animDone += delta;
        } else {
            if(setter) setter(to); //Protect from large animations
        }
    }

    // https://github.com/acron0/Easings/blob/master/Easings.cs
    namespace interpolator {

        /// Linear (aka no modification) interpolator
        template<typename T = float>
        struct Linear: public Interpolator<T> {
            constexpr T interpolate(T perc) const override { return perc; }
        };

        /// Quadratic interpolator (`x^2`) that starts slow and ends fast
        template<typename T = float>
        struct QuadIn: public Interpolator<T> {
            constexpr T interpolate(T perc) const override { return perc * perc; }
        };

        /// Quadratic interpolator (`x^2`) that starts fast and ends slow
        template<typename T = float>
        struct QuadOut: public Interpolator<T> {
            constexpr T interpolate(T perc) const override { return -perc * (perc-T(2)); }
        };

        /// Quadratic interpolator (`x^2`) that starts slow, continues fast and ends slow
        template<typename T = float>
        struct QuadInOut: public Interpolator<T> {
            constexpr T interpolate(T perc) const override {
                perc *= T(2);
                if(perc < T(1)) return T(0.5) * perc * perc;
                perc -= T(2);
                return T(0.5) * (perc * perc + T(2));
            }
        };

        /// Cubic interpolator (`x^3`) that starts slow and ends fast
        template<typename T = float>
        struct CubicIn: public Interpolator<T> {
            constexpr T interpolate(T perc) const override { return perc * perc * perc; }
        };

        /// Cubic interpolator (`x^3`) that starts fast and ends slow
        template<typename T = float>
        struct CubicOut: public Interpolator<T> {
            constexpr T interpolate(T perc) const override {
                perc -= T(1);
                return 1 + perc * perc * perc;
            }
        };

        /// Cubic interpolator (`x^3`) that starts slow, continues fast and ends slow
        template<typename T = float>
        struct CubicInOut: public Interpolator<T> {
            constexpr T interpolate(T perc) const override {
                if((perc *= T(2)) < T(1)) return T(0.5) * perc * perc * perc;
                perc -= T(2);
                return T(0.5) * (perc * perc * perc + T(2));
            }
        };

        /// Quart interpolator (`x^4`) that starts slow and ends fast
        template<typename T = float>
        struct QuartIn: public Interpolator<T> {
            constexpr T interpolate(T perc) const override { return perc * perc * perc * perc; }
        };

        /// Quart interpolator (`x^4`) that starts fast and ends slow
        template<typename T = float>
        struct QuartOut: public Interpolator<T> {
            constexpr T interpolate(T perc) const override { return -(perc -= T(1)) * perc * perc * perc + T(1); }
        };

        /// Quart interpolator (`x^4`) that starts slow, continues fast and ends slow
        template<typename T = float>
        struct QuartInOut: public Interpolator<T> {
            constexpr T interpolate(T perc) const override {
                if((perc *= T(2)) < T(1)) return T(0.5) * perc * perc * perc * perc;
                perc -= T(2);
                return T(0.5) * (perc * perc * perc * perc + T(2));
            }
        };

        /// Quint interpolator (`x^5`) that starts slow and ends fast
        template<typename T = float>
        struct QuintIn: public Interpolator<T> {
            constexpr T interpolate(T perc) const override { return perc * perc * perc * perc * perc; }
        };

        /// Quint interpolator (`x^5`) that starts fast and ends slow
        template<typename T = float>
        struct QuintOut: public Interpolator<T> {
            constexpr T interpolate(T perc) const override { return -(perc -= T(1)) * perc * perc * perc * perc + T(1); }
        };

        /// Quint interpolator (`x^5`) that starts slow, continues fast and ends slow
        template<typename T = float>
        struct QuintInOut: public Interpolator<T> {
            constexpr T interpolate(T perc) const override {
                if((perc *= T(2)) < T(1)) return T(0.5) * perc * perc * perc * perc * perc;
                perc -= T(2);
                return T(0.5) * (perc * perc * perc * perc * perc + T(2));
            }
        };

        /// Sine interpolator (`cos(x)`) that starts slow and ends fast (in fact, here uses a `cosine`)
        template<typename T = float>
        struct SineIn: public Interpolator<T> {
            static constexpr T PI = T(3.1415926535897932385L);
            constexpr T interpolate(T perc) const override { return -std::cos(perc * (PI / T(2))) + 1; }
        };

        /// Sine interpolator (`sin(x)`) that fast slow and ends slow (true `sine`)
        template<typename T = float>
        struct SineOut: public Interpolator<T> {
            static constexpr T PI = T(3.1415926535897932385L);
            constexpr T interpolate(T perc) const override { return std::sin(perc * (PI / T(2))); }
        };

        /// Sine interpolator (`cos(x)`) that starts slow, continues fast and ends slow (again, uses a `cosine`, but it's the same)
        template<typename T = float>
        struct SineInOut: public Interpolator<T> {
            static constexpr T PI = T(3.1415926535897932385L);
            constexpr T interpolate(T perc) const override { return -T(0.5) * (std::cos(PI * perc) - 1); }
        };

        /// An interpolator that goes back and the goes fast to its final position
        template<typename T = float>
        struct BackIn: public Interpolator<T> {
            static constexpr T S = T(1.70158);
            constexpr T interpolate(T perc) const override { return perc * perc * ((S + 1) * perc - S); }
        };

        /// An interpolator that starts fast, goes away its end and goes backs slowly to its end
        template<typename T = float>
        struct BackOut: public Interpolator<T> {
            static constexpr T S = T(1.70158);
            constexpr T interpolate(T perc) const override { return (perc -= 1) * perc * ((S + 1) * perc + S) + 1; }
        };

        /// A combination of BackIn and BackOut
        template<typename T = float>
        struct BackInOut: public Interpolator<T> {
            static constexpr T S = T(1.70158);
            constexpr T interpolate(T t) const override {
                if((t *= T(2)) < 1) return T(0.5) * (t * t * (((S * T(1.525)) + T(1)) * t - S * T(1.525)));
                return T(0.5) * ((t -= 2) * t * (((S * T(1.525)) +T(1)) * t + S * T(1.525)) + T(2));
            }
        };

        /// An interpolator that starts slowly and increments its speed incrediblement fast
        template<typename T = float>
        struct CircIn: public Interpolator<T> {
            constexpr T interpolate(T perc) const override { return -std::sqrt(T(1) - perc * perc) - T(1); }
        };

        /// An interpolator that starts fast and decrements its speed incrediblement slowly
        template<typename T = float>
        struct CircOut: public Interpolator<T> {
            constexpr T interpolate(T perc) const override { return std::sqrt(T(1) - (perc -= T(1)) * perc); }
        };

        /// An interpolator that starts and ends slowly, and in intermediate positions goes super-fast
        template<typename T = float>
        struct CircInOut: public Interpolator<T> {
            constexpr T interpolate(T perc) const override {
                return (perc *= T(2)) < 1 ? -T(0.5) * std::sqrt(T(1) - perc * perc) - T(1) : T(1) * (std::sqrt(T(1) - (perc -= T(2)) * perc) + T(1));
            }
        };

        /// Interpolator that simulates a bounce at the top of something
        template<typename T = float>
        struct BounceOut: public Interpolator<T> {
            constexpr T interpolate(T perc) const override {
                if(perc < T(1)/T(2.75)) {
                    return T(7.5625) * perc * perc;
                } else if(perc < T(2)/T(2.75)) {
                    return T(7.5625) * (perc -= T(1.5)/T(2.75)) * perc + T(0.75);
                } else if(perc < T(2.5)/T(2.75)) {
                    return T(7.5625) * (perc -= T(2.25)/T(2.75)) * perc + T(0.9375);
                } else {
                    return T(7.5625) * (perc -= T(2.625)/T(2.75)) * perc + T(0.984375);
                }
            }
        };

        /// Interpolator that simulates a bounce at the bottom of something
        template<typename T = float>
        struct BounceIn: public Interpolator<T> {
            constexpr T interpolate(T perc) const override { return T(1) - BounceOut<T>().interpolate(1-perc); }
        };

        /// Starts being a BounceIn and by a strange sittuation, the gravity changes and becomes a BounceOut
        template<typename T = float>
        struct BounceInOut: public Interpolator<T> {
            constexpr T interpolate(T perc) const override {
                return perc < T(0.5) ? BounceIn<T>().interpolate(perc * T(2)) * T(0.5) : T(0.5) + T(0.5) * BounceOut<T>().interpolate(T(2)*perc - T(1));
            }
        };

        /// Simulates something eslastic, but in an inverted way
        template<typename T = float>
        struct ElasticIn: public Interpolator<T> {
            static constexpr T PI = T(3.1415926535897932385L);
        protected: T a, p;
        public:

            constexpr ElasticIn() {
                p = T(0.3);
                a = 1;
            }

            constexpr T interpolate(T t) const override {
                if(t == T(0) || t == T(1)) return t;
                T s = p / T(2*PI) * std::asin(T(1) / a);
                return -(a * std::pow(T(2), -T(10) * (t -= T(1))) * std::sin((t - s) * T(2*PI) / p));
            }
        };

        /// Simulates something eslastic
        template<typename T = float>
        struct ElasticOut: public Interpolator<T> {
            static constexpr T PI = T(3.1415926535897932385L);
        protected: T a, p;
        public:

            constexpr ElasticOut() {
                p = T(0.3);
                a = 1;
            }

            constexpr T interpolate(T t) const override {
                if(t == T(0) || t == T(1)) return t;
                T s = p / T(2*PI) * std::asin(T(1) / a);
                return a * std::pow(T(2), -T(10) * t) * std::sin((t - s) * T(2*PI) / p) + T(1);
            }
        };

        /// Swapes positions with some bounce, like elastic
        template<typename T = float>
        struct ElasticInOut: public Interpolator<T> {
            static constexpr T PI = T(3.1415926535897932385L);
        protected: T a, p;
        public:

            constexpr ElasticInOut() {
                p = T(0.45);
                a = 1;
            }

            constexpr T interpolate(T t) const override {
                if(t == T(0) || (t *= T(2)) == T(2)) return t;
                T s = p / T(2*PI) * std::asin(T(1) / a);
                if(t < 1) return -T(0.5) * (a * std::pow(T(2), T(10) * (t -= T(1)) * std::sin((t - s) * T(2*PI) / p)));
                else return a * std::pow(T(2), -T(10) * (t -= T(1))) * std::sin((t - s) * T(2*PI) / p) * T(0.5) + T(1);
            }
        };

        /// Exponential interpolator (`e^x`) that starts slow and ends fast
        template<typename T = float>
        struct ExpoIn: public Interpolator<T> {
            constexpr T interpolate(T t) const override { return (t == T(0)) ? T(0) : std::pow(T(2), T(10) * (t - 1)); }
        };

        /// Exponential interpolator (`e^x`) that starts fast and ends slow
        template<typename T = float>
        struct ExpoOut: public Interpolator<T> {
            constexpr T interpolate(T t) const override { return (t == T(1)) ? T(1) : -std::pow(T(2), -T(10) * t) + T(1); }
        };

        /// Exponential interpolator (`e^x`) that starts slow, continues fast and ends slow
        template<typename T = float>
        struct ExpoInOut: public Interpolator<T> {
            constexpr T interpolate(T t) const override {
                if(t == T(0) || t == T(1)) return t;
                return (t *= T(2)) < T(1) ? T(0.5) * std::pow(T(2), T(10) * (t - T(1))) : T(0.5) * (-std::pow(T(2), -T(10) * --t) + T(2));
            }
        };

    }

}
