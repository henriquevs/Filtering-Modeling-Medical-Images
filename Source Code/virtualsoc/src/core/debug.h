#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DIE(a) exit(a);

#define ASSERT(cond) if (!(cond))                                                      \
                     {                                                                 \
                       fprintf(stderr,                                                 \
                       "Assert failed [%s] on file %s at line %d at time %10.1f ns\n", \
                        #cond, __FILE__, __LINE__, sc_simulation_time());              \
                        DIE(0xBADF00D);                                                \
                     }

#endif // __DEBUG_H__

