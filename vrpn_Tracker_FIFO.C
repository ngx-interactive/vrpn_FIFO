#include "vrpn_Tracker_FIFO.h"

//#if defined(VRPN_USE_FIFO)

#include <iostream>
#include <string>

#include<ios> //used to get stream size
#include<limits> //used to get numeric limits

#include <boost/iostreams/device/file_descriptor.hpp>
//#include <boost/iostreams/stream.hpp>
#include <boost/asio.hpp>

//#include <boost/asio/io_service.hpp>
//#include <boost/asio/streambuf.hpp>
//#include <boost/asio/read_until.hpp>

#include "quat.h"

#include "vrpn_SendTextMessageStreamProxy.h"

#include <csignal>

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
bool vrpn_Tracker_FIFO::_parse_tracker_data()
{
	int id;
  float pos[3];

  boost::asio::io_context io_context;
  boost::asio::streambuf buffer;

  int fifo_d = open("/tmp/test", O_RDONLY);
  boost::asio::posix::stream_descriptor fifo(io_context, fifo_d);

	boost::asio::async_read_until(fifo, buffer, "\n",
    [&](const boost::system::error_code& error, std::size_t bytes_transferred) {
      std::istream is(&buffer);
      std::string line;

      //std::string const file(boost::asio::buffer_cast<char const*>(buffer.data()));
      while (std::getline(is, line)) {
        //printf("line: %s\n", line.c_str());

        if (sscanf(line.c_str(), "%d %f %f %f", &id, &pos[0], &pos[1], &pos[2]) != 4) {
          //fprintf(stderr, "Bad vrpn_Tracker_FIFO input: %s\n", line.c_str());
        } else {
          //fprintf(stderr, "Good vrpn_Tracker_FIFO for ID: %d\n", id);

          report_changes(pos, id);
        }
      }
		}
	);
  io_context.run();
	//close(fifo_d);


  return true;

  //}
}

void vrpn_Tracker_FIFO::report_changes(float pos[], int id) {
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

