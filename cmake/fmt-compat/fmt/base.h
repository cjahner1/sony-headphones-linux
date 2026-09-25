#pragma once

// fmt 9 (Ubuntu 24.04) has no fmt/base.h. The MDR sources only require the
// public formatting API, which is supplied by format.h in both fmt 9 and fmt 12.
#include <fmt/format.h>
