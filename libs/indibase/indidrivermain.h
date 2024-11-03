#pragma once
#ifdef INDI_AS_LIBRARY
#ifdef __cplusplus
extern "C" {
#endif
int indi_driver_event_loop_main_stop();
int indi_driver_event_loop_main(int input_pipe_fd,int output_pipe_fd);
#ifdef __cplusplus
}
#endif
#endif
