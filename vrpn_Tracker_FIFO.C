#include "vrpn_Tracker_FIFO.h"

//#if defined(VRPN_USE_FIFO)

#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>


#include "quat.h"

#include "vrpn_SendTextMessageStreamProxy.h"

#include <csignal>

#define BUFSIZE 512

vrpn_Tracker_FIFO::vrpn_Tracker_FIFO(const char* name,vrpn_Connection* c,int id) :
	vrpn_Tracker(name, c)
{
	fprintf(stderr, "vrpn_Tracker_FIFO : Device %s listen for ID %d\n", name, id);
}

void vrpn_Tracker_FIFO::mainloop()
{
  server_mainloop();
  //std::thread stdinthread(&vrpn_Tracker_FIFO::_parse_tracker_data, this);

  vrpn_Tracker_FIFO::_parse_tracker_data();
  //float pos[3] = {1., 1., 1.};
  //report_changes(pos, 0);
}


/**
 * Lorem
 */
bool vrpn_Tracker_FIFO::_parse_tracker_data() {
  HANDLE hPipe;
  BOOL fSuccess = FALSE; 
  LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\szgmtrack"); 
  DWORD  cbRead, cbToWrite, cbWritten, dwMode;

  char input[BUFSIZE];

  int id;
  double pos[3];

  while (1) {
    hPipe = CreateFile(lpszPipename,
                        GENERIC_READ |  // read and write access 
                        GENERIC_WRITE,
                        0,             // no sharing
                        NULL,          // default security attributes
                        OPEN_EXISTING, // opens existing pipe
                        0,             // default attributes
                        NULL);         // no template file

    // Break if the pipe handle is valid.

    if (hPipe != INVALID_HANDLE_VALUE) break;

    // Exit if an error other than ERROR_PIPE_BUSY occurs.

    if (GetLastError() != ERROR_PIPE_BUSY) {
      //_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
      return false;
    }

    // All pipe instances are busy, so wait for 20 seconds.

    if (!WaitNamedPipe(lpszPipename, 20000)) {
      printf("Could not open pipe: 20 second wait timed out.");
      return false;
    }
  }

  // The pipe connected; change to message-read mode.
  dwMode = PIPE_READMODE_MESSAGE;
  fSuccess = SetNamedPipeHandleState(hPipe,   // pipe handle
                                      &dwMode, // new pipe mode
                                      NULL,    // don't set maximum bytes
                                      NULL);   // don't set maximum time
  if (!fSuccess) {
    _tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"),
              GetLastError());
    return false;
  }

  // Read
  while (true) {
    // Read from the pipe.
    fSuccess = ReadFile(hPipe,                   // pipe handle
                        &input,                   // buffer to receive reply
                        BUFSIZE * sizeof(TCHAR), // size of buffer
                        &cbRead,                 // number of bytes read
                        NULL);                   // not overlapped

    //_tprintf(TEXT("fSuccess=%d\n"), fSuccess);

    if (!fSuccess && GetLastError() != ERROR_MORE_DATA) break;


    // Loop through lines
    char * curLine = input;

    while(curLine) {
      char * nextLine = strchr(curLine, '\n');
      if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

      //printf("curLine=[%s]\n", curLine);

      if (sscanf(curLine, "%d %lf %lf %lf", &id, &pos[0], &pos[1], &pos[2]) != 4) {
        printf("Bad vrpn_Tracker_FIFO input: %s\n", input);
      } else {
        //printf("Good vrpn_Tracker_FIFO for ID: %d\n", id);

        //printf("Pos: %lf", pos[0]);
        //printf(" %lf", pos[1]);
        //printf(" %lf\n", pos[2]);

        report_changes(pos, id);
      }

      if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy    
      curLine = nextLine ? (nextLine+1) : NULL;
    }

  }

  if (!fSuccess) {
    _tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
    return false;
  }

  printf("\n<End of message, press ENTER to terminate connection and exit>");
  _getch();

  CloseHandle(hPipe);

  return true;
}

void vrpn_Tracker_FIFO::report_changes(double pos[], int id) {
  this->pos[0] = pos[0];
  this->pos[1] = pos[1];
  this->pos[2] = pos[2];
  //this->d_sensor = id;

  // report trackerchanges
  struct timeval ts;
  vrpn_gettimeofday(&ts, NULL);

  // from vrpn_Tracker_DTrack::dtrack2vrpnbody
  if (d_connection) {
    char msgbuf[1000];
    // Encode pos and d_quat
    int len = vrpn_Tracker::encode_to(msgbuf);
    //std::cout << d_sender_id << std::endl;
    if (d_connection->pack_message(len, ts, position_m_id, id + 1,
                                    msgbuf, vrpn_CONNECTION_LOW_LATENCY)) {
      // error
      return;
    }
    // fprintf(stderr, "Packed and sent\n");
  }
}

//#endif // defined VRPN_USE_FIFO

