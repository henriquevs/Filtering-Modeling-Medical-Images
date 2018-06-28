#ifndef __SIM_SUPPORT_H__
#define __SIM_SUPPORT_H__

#include <systemc.h>
#include "globals.h"

class Sim_Support
{
  public:
    Sim_Support(int argc, char *argv[], char *envp[]);
    ~Sim_Support();
    
    bool catch_sim_message(uint32_t addr, uint32_t *data, bool write, uint8_t ID);
    double *t1,*t2,*t3,*t_idle,*t_exec, t1_cl, t2_cl, t_exec_cl;
    bool is_first;
    int id_first, done_metrics;
    
  private:
    bool *stopped_time;
    bool *stopped_cycle;
    uint64_t *current_time;
    uint64_t *current_cycle;
    char **debug_msg_string;
    uint32_t *debug_msg_value;
    uint32_t *debug_msg_mode;
    uint32_t *debug_msg_id;
    int argc;
    char **argv, **envp;
    bool *file_read;
    FILE **current_file;
    uint32_t *file_window_valid_data;
    uint32_t **filebuffer;
    char **filename;
};

extern Sim_Support *simsuppobject;

#endif // __SIM_SUPPORT_H__
