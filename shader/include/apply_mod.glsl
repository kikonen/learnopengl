#ifdef USE_FLAG_WAVE
#ifdef USE_TBN
    mod_flag_wave(pos, normal, tangent);
#else
{
    vec3 tangent;
    mod_flag_wave(pos, normal, tangent);
}
#endif
#endif
#ifdef USE_TREE_WIND
#ifdef USE_TBN
    mod_tree_wind(pos, normal, tangent);
#else
{
    vec3 tangent;
    mod_tree_wind(pos, normal, tangent);
}
#endif
#endif
