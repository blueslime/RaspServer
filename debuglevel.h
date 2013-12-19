#ifndef DEBUG_LEVEL_H
#define DEBUG_LEVEL_H

#define DEBUG_LEVEL_0

#ifdef DEBUG_LEVEL_0
#undef DEBUG_LEVEL_1
#undef DEBUG_LEVEL_2
#endif //LEVEL_2

#ifdef DEBUG_LEVEL_1
#undef DEBUG_LEVEL_2
#define DEBUG_LEVEL_0
#endif //LEVEL_1

#ifdef DEBUG_LEVEL_2
#define DEBUG_LEVEL_1
#define DEBUG_LEVEL_0
#endif //LEVEL_0

#endif //DEBUG_LEVEL_H

