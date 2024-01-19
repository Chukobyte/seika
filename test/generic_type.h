#pragma once

#define GENERIC_TYPE_GET_NAME(TYPE_NAME) \
private_##TYPE_NAME##_get_name()

#define GENERIC_TYPE_GET_INDEX(TYPE_NAME) \
private_##TYPE_NAME##_get_index()
