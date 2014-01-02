sxshacker
=========

find the correct runtime/manifest/sxs for Windows software for software developer.


People who want to use SxSHacker, need to define this in Visual Studio Projects:
  _BIND_TO_CURRENT_VCLIBS_VERSION=1

Or different dlls will be loaded in different systems for different environments.

Developer always complain that their software can't run at users' computer. Once run, the system popups one MessageBox, which says:
  This application has failed to start because the application configuration is incorrect. Reinstalling the application may fix the problem

In most case, that means you need to publish sxs or crt or assembly or side-by-side stuff with your software.
  
SxSHacker is used to find all proper stuff for you software.
