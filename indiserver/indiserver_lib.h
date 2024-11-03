#pragma once
#ifdef INDI_AS_LIBRARY
#include <vector>
#include <string>
///
/// \bried  RUN indi server as service inside the process 
/// 
/// this functions requires its own thread to run. You can call detach on the
/// thread
/// 
/// Each string is either remote driver containing '@' in it or a path to a shared
/// object to load
///
/// NOTE: it does not run class/standard executable indi drviers
///
int indiserver_main(std::vector<std::string> drivers_to_load);

///
/// Stop the server
///
void indiserver_main_shutdown();

///
/// Prevennt libev main loop to handle sigchld event by presetting sigaction of SIGCHLD to SIG_DFL
///
/// Should be called after indiserver_main is running
///
void indiserver_main_reset_sigchld();

#endif
