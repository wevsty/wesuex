#pragma once

#define ARRAY_SIZE(_) ((sizeof (_) / sizeof (*_)))
#define STRING_LEN_NO_ZERO(_) (((sizeof (_) / sizeof (*_)) - 1))