/*
 * $Id: svcproc.h,v 1.5 1996/01/28 02:55:02 robertj Exp $
 *
 * Portable Windows Library
 *
 * Service Process for Windows NT platforms
 *
 * Copyright 1995 Equivalence
 *
 * $Log: svcproc.h,v $
 * Revision 1.5  1996/01/28 02:55:02  robertj
 * WIN16 support.
 *
 * Revision 1.4  1996/01/02 12:54:12  robertj
 * Made "common".
 *
 * Revision 1.3  1995/12/10 11:50:05  robertj
 * Numerous fixes for WIN32 service processes.
 *
 * Revision 1.2  1995/07/02 01:23:27  robertj
 * Set up service process to be in subthread not main thread.
 *
 * Revision 1.1  1995/06/17 00:50:54  robertj
 * Initial revision
 *
 */


#ifndef _PSERVICEPROCESS


///////////////////////////////////////////////////////////////////////////////
// PServiceProcess

#include "../../common/svcproc.h"
#ifdef _WIN32
  private:
    static void __stdcall StaticMainEntry(DWORD argc, LPTSTR * argv);
    /* Internal function called from the Service Manager. This simply calls the
       <A>MainEntry()</A> function on the PServiceProcess instance.
    */

    void MainEntry(DWORD argc, LPTSTR * argv);
    /* Internal function function that takes care of actually starting the
       service, informing the service controller at each step along the way.
       After launching the worker thread, it waits on the event that the worker
       thread will signal at its termination.
    */

    static DWORD EXPORTED StaticThreadEntry(LPVOID);
    /* Internal function called to begin the work of the service process. This
       essentially just calls the <A>Main()</A> function on the
       PServiceProcess instance.
    */

    void ThreadEntry();
    /* Internal function function that starts the worker thread for the
       service.
    */

    static void __stdcall StaticControlEntry(DWORD code);
    /* This function is called by the Service Controller whenever someone calls
       ControlService in reference to our service.
     */

    void ControlEntry(DWORD code);
    /* This function is called by the Service Controller whenever someone calls
       ControlService in reference to our service.
     */

    BOOL ReportStatus(
      DWORD dwCurrentState,
      DWORD dwWin32ExitCode = NO_ERROR,
      DWORD dwCheckPoint = 0,
      DWORD dwWaitHint = 0
    );
    /* This function is called by the Main() and Control() functions to update the
       service's status to the service control manager.
     */


    enum ProcessCommandResult {
      DebugCommandMode, ProcessCommandError, CommandProcessed
    };
    ProcessCommandResult ProcessCommand(const char * cmd);
    // Process command line argument for controlling the service.


    SERVICE_STATUS        status;
    SERVICE_STATUS_HANDLE statusHandle;
    HANDLE                terminationEvent;

#endif

  friend void PAssertFunc(const char * file, int line, const char * msg);
};


#if defined(_WIN32) || !defined(_WINDLL)

inline PServiceProcess * PServiceProcess::Current()
  { return (PServiceProcess *)PProcessInstance; }

#endif


#endif
