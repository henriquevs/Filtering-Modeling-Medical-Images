#ifndef __HWS_SUPPORT_H__
#define __HWS_SUPPORT_H__

#define assert(x) \
  do { \
      if (!(x)) \
      { \
	abort();\
      } \
  } while (0)

//event-programming API
inline int maybe_get_event(unsigned int NSL);

inline int get_event(unsigned int NSL);

inline void free_event(int EVT_ID);

inline void release(int EVT_ID, unsigned int BMSK);



//slave API
inline void notify(int EVT_ID);

inline void notify_and_set(int EVT_ID);


#define MY_SLAVE_PORT(ID)	(ID & 3)  //Assign a slaves to a certain HWS slave port depending on its ID (ID % 4)

#define PROG_PORT		0
  


#endif /* __HWS_SUPPORT_H__ */
