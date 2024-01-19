#ifdef SKA_GENERIC_TYPE

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define GENERIC_FUNC(FUNC_NAME) CONCAT(private_, CONCAT(SKA_GENERIC_TYPE, _##FUNC_NAME))

#define STRINGIZE(x) #x
#define WRAP_IN_QUOTES(x) STRINGIZE(x)

const char* GENERIC_FUNC(get_name)() {
    static const char* genericName = WRAP_IN_QUOTES(SKA_GENERIC_TYPE);
    return genericName;
}

size_t GENERIC_FUNC(get_index)() {
    return 0;
}

#undef GENERIC_FUNC
#undef WRAP_IN_QUOTES
#undef CONCAT
#undef CONCAT_IMPL
#undef STRINGIZE

#endif // SKA_GENERIC_TYPE
