#pragma once

// https://adrianb.io/2014/08/09/perlinnoise.html
// https://www.scratchapixel.com/code.php?id=57&origin=/lessons/procedural-generation-virtual-worlds/perlin-noise-part-2
class Perlin final
{
public:
    Perlin(int repeat = -1);

    double perlin(double x, double y, double z);
    double octavePerlin(double x, double y, double z, int octaves, double persistence);

private:
    int inc(int num);

    static double grad(int hash, double x, double y, double z);
    static double fade(double t);
    static double lerp(double a, double b, double x);

private:
    int repeat;
};


