#pragma once
#include <cmath>

//イージング関数 一覧
class easing
{
    static constexpr float PI = 3.14159265f;

    //Ctime(current time):現在の時間
    //start:始点
    //end:終点
    //d(duration):効果時間

    //https://easings.net/ja

public:

    struct Sine
    {
        static float easeIn(float Ctime, float start, float end, float d)
        {
            return -end * cos(Ctime / d * (PI / 2.0f)) + end + start;
        }
        float easeOut(float Ctime, float start, float end, float d)
        {
            return end * sin(Ctime / d * (PI / 2.0f)) + start;
        }
        float easeInOut(float Ctime, float start, float end, float d)
        {
            return -end / 2.0f * (cos(PI * Ctime / d) - 1.0f) + start;
        }
    };

    struct Quint
    {
        static float easeIn(float Ctime, float start, float end, float d)
        {
            Ctime /= d;
            return end * Ctime * Ctime * Ctime * Ctime * Ctime + start;
        }
        static float easeOut(float Ctime, float start, float end, float d)
        {
            (Ctime = Ctime / d - 1.0f);
            return end * (Ctime * Ctime * Ctime * Ctime * Ctime + 1.0f) + start;
        }

        static float easeInOut(float Ctime, float start, float end, float d)
        {
            (Ctime /= d / 2.0f);
            if (Ctime < 1.0f)
                return end / 2.0f * Ctime * Ctime * Ctime * Ctime * Ctime + start;
            (Ctime -= 2.0f);
            return end / 2.0f * (Ctime * Ctime * Ctime * Ctime * Ctime + 2.0f) + start;
        }
    };

    struct Quart
    {
        static float easeIn(float Ctime, float start, float end, float d)
        {
            //c -= d;
            Ctime /= d;
            //return c * (t) * t * t * t + b;
            return end * (Ctime) *Ctime * Ctime * Ctime + start;

        }
        static float easeOut(float Ctime, float start, float end, float d)
        {
            Ctime = Ctime / d - 1.0f;
            return -end * (Ctime * Ctime * Ctime * Ctime - 1.0f) + start;
        }

        static float easeInOut(float Ctime, float start, float end, float d)
        {
            Ctime /= d / 2.0f;
            if (Ctime < 1.0f)
                return end / 2.0f * Ctime * Ctime * Ctime * Ctime + start;
            Ctime -= 2.0f;
            return -end / 2.0f * (Ctime * Ctime * Ctime * Ctime - 2.0f) + start;
        }
    };

    struct Quad
    {
        static float easeIn(float Ctime, float start, float end, float d)
        {
            Ctime /= d;
            return end * Ctime * Ctime + start;
        }
        static float easeOut(float Ctime, float start, float end, float d)
        {
            Ctime /= d;
            return -end * Ctime * (Ctime - 2.0f) + start;
        }

        static float easeInOut(float Ctime, float start, float end, float d)
        {
            Ctime /= d / 2.0f;
            if (Ctime < 1.0f)
                return ((end / 2.0f) * (Ctime * Ctime)) + start;
            return -end / 2.0f * (((Ctime - 2.0f) * (--Ctime)) - 1.0f) + start;
            /*
            originally return -c/2 * (((t-2)*(--t)) - 1) + b;

            I've had to swap (--t)*(t-2) due to diffence in behaviour in
            pre-increment operators between java and c++, after hours
            of joy
            */

        }
    };

    struct Linear
    {
        static float easeNone(float Ctime, float start, float end, float d)
        {
            return end * Ctime / d + start;
        }
        static float easeIn(float Ctime, float start, float end, float d)
        {
            return end * Ctime / d + start;
        }
        static float easeOut(float Ctime, float start, float end, float d)
        {
            return end * Ctime / d + start;
        }

        static float easeInOut(float Ctime, float start, float end, float d)
        {
            return end * Ctime / d + start;
        }
    };

    struct Expo
    {
        float easeIn(float Ctime, float start, float end, float d)
        {
            return (Ctime == 0.0f) ? start : end * pow(2.0f, 10.0f * (Ctime / d - 1.0f)) + start;
        }
        float easeOut(float Ctime, float start, float end, float d)
        {
            return (Ctime == d) ? start + end : end * (-pow(2.0f, -10.0f * Ctime / d) + 1.0f) + start;
        }

        static  float easeInOut(float Ctime, float start, float end, float d)
        {
            if (Ctime == 0) return start;
            if (Ctime == d) return start + end;
            if ((Ctime /= d / 2.0f) < 1.0f) return end / 2.0f * pow(2.0f, 10.0f * (Ctime - 1.0f)) + start;
            return end / 2.0f * (-pow(2.0f, -10.0f * --Ctime) + 2.0f) + start;
        }
    };

    struct Elastic
    {
        static float easeIn(float Ctime, float start, float end, float d)
        {
            if (Ctime == 0.0f) return start;
            if ((Ctime /= d) == 1.0f) return start + end;
            float p = d * .3f;
            float a = end;
            float s = p / 4.0f;
            float postFix = a * pow(2.0f, 10.0f * (Ctime -= 1.0f)); // this is a fix, again, with post-increment operators
            return -(postFix * sin((Ctime * d - s) * (2.0f * PI) / p)) + start;
        }

        static float easeOut(float Ctime, float start, float end, float d)
        {
            if (Ctime == 0) return start;
            if ((Ctime /= d) == 1.0f) return start + end;
            float p = d * .3f;
            float a = end;
            float s = p / 4.0f;
            return (a * pow(2.0f, -10.0f * Ctime) * sin((Ctime * d - s) * (2.0f * PI) / p) + end + start);
        }

        static float easeInOut(float Ctime, float start, float end, float d)
        {
            if (Ctime == 0) return start;
            if ((Ctime /= d / 2.0f) == 2.0f) return start + end;
            float p = d * (.3f * 1.5f);
            float a = end;
            float s = p / 4.0f;

            if (Ctime < 1.0f)
            {
                float postFix = a * pow(2.0f, 10.0f * (Ctime -= 1.0f)); // postIncrement is evil
                return -.5f * (postFix * sin((Ctime * d - s) * (2.0f * PI) / p)) + start;
            }
            float postFix = a * pow(2.0f, -10.0f * (Ctime -= 1.0f)); // postIncrement is evil
            return postFix * sin((Ctime * d - s) * (2.0f * PI) / p) * .5f + end + start;
        }
    };

    struct Cubic
    {
        static float easeIn(float Ctime, float start, float end, float d)
        {
            (Ctime /= d);
            return end * Ctime * Ctime * Ctime + start;
        }
        static float easeOut(float Ctime, float start, float end, float d)
        {
            return end * ((Ctime = Ctime / d - 1.0f) * Ctime * Ctime + 1.0f) + start;
        }

        static float easeInOut(float Ctime, float start, float end, float d)
        {
            if ((Ctime /= d / 2.0f) < 1.0f)
                return end / 2.0f * Ctime * Ctime * Ctime + start;
            return end / 2.0f * ((Ctime -= 2.0f) * Ctime * Ctime + 2.0f) + start;
        }
    };

    struct Circ
    {
        static float easeIn(float Ctime, float start, float end, float d)
        {
            return -end * (sqrt(1.0f - (Ctime /= d) * Ctime) - 1.0f) + start;
        }
        static float easeOut(float Ctime, float start, float end, float d)
        {
            return end * sqrt(1.0f - (Ctime = Ctime / d - 1.0f) * Ctime) + start;
        }

        static float easeInOut(float Ctime, float start, float end, float d)
        {
            if ((Ctime /= d / 2.0f) < 1.0f) return -end / 2.0f * (sqrt(1.0f - Ctime * Ctime) - 1.0f) + start;
            return end / 2.0f * (sqrt(1.0f - Ctime * (Ctime -= 2.0f)) + 1.0f) + start;
        }
    };

    struct Bounce
    {
        float easeIn(float Ctime, float start, float end, float d)
        {
            return end - easeOut(d - Ctime, 0, end, d) + start;
        }
        float easeOut(float Ctime, float start, float end, float d)
        {
            if ((Ctime /= d) < (1.0f / 2.75f))
            {
                return end * (7.5625f * Ctime * Ctime) + start;
            }
            else if (Ctime < (2.0f / 2.75f))
            {
                float postFix = Ctime -= (1.5f / 2.75f);
                return end * (7.5625f * (postFix)*Ctime + .75f) + start;
            }
            else if (Ctime < (2.5f / 2.75f))
            {
                float postFix = Ctime -= (2.25f / 2.75f);
                return end * (7.5625f * (postFix)*Ctime + .9375f) + start;
            }
            else {
                float postFix = Ctime -= (2.625f / 2.75f);
                return end * (7.5625f * (postFix)*Ctime + .984375f) + start;
            }
        }

        float easeInOut(float Ctime, float start, float end, float d)
        {
            if (Ctime < d / 2.0f) return easeIn(Ctime * 2.0f, 0, end, d) * .5f + start;
            else return easeOut(Ctime * 2.0f - d, 0, end, d) * .5f + end * .5f + start;
        }
    };

    struct Back
    {
        float easeIn(float Ctime, float start, float end, float d)
        {
            float s = 1.70158f;
            float postFix = Ctime /= d;
            return end * (postFix)*Ctime * ((s + 1.0f) * Ctime - s) + start;
        }
        float easeOut(float Ctime, float start, float end, float d)
        {
            float s = 1.70158f;
            return end * ((Ctime = Ctime / d - 1.0f) * Ctime * ((s + 1.0f) * Ctime + s) + 1.0f) + start;
        }

        float easeInOut(float Ctime, float start, float end, float d)
        {
            float s = 1.70158f;
            if ((Ctime /= d / 2.0f) < 1.0f) return end / 2.0f * (Ctime * Ctime * (((s *= (1.525f)) + 1.0f) * Ctime - s)) + start;
            float postFix = Ctime -= 2.0f;
            return end / 2.0f * ((postFix)*Ctime * (((s *= (1.525f)) + 1.0f) * Ctime + s) + 2.0f) + start;
        }
    };
};
