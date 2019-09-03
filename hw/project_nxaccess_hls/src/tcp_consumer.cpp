//--------------------------------------------------------------------------------
//--! Enyx Confidential
//--!
//--! Organization:          Enyx
//--! Project Identifier:    010 - Enyx nxAccess HLS Framework
//--! Author:                Raphael Charolois (raphael.charolois@enyx.com)
//--!
//--! © Copyright            Enyx 2019
//--! © Copyright Notice:    The source code for this program is not published or otherwise divested of its trade secrets,
//--!                        irrespective of what has been deposited with the U.S. Copyright Office.
//--------------------------------------------------------------------------------

#include <iostream>

#include "tcp_consumer.hpp"

namespace enyx {
namespace oe {
namespace nxaccess_hw_algo {

void TcpConsumer::p_consume_tcp(hls::stream<enyx::oe::hwstrat::tcp_reply_payload> & tcp_replies_in,
                                hls::stream<enyx::oe::hwstrat::trigger_command_axi> &output) {

    #pragma HLS INLINE recursive
    #pragma HLS PIPELINE enable_flush
    if (!tcp_replies_in.empty()) { // there's some TCP data to read
        enyx::oe::hwstrat::tcp_reply_payload tcp_data;
        tcp_data = tcp_replies_in.read();
        // below is example code that shows how to identify the session, and modify the data received.
        // a complete working use case should process the data, and transmit some information to
        // the main algorithm.

        // Let as example : parse the FIX protocol or any other protocol
        // It is to be implemented with a real use case.
        if(tcp_data.user(7,0) == 0) {  // if tcp.session_id == 0
            // do something for session #0
            std::cout << "[TCP_CONSUMER] Received data for session id #0 !" << std::endl;
        } else if (tcp_data.user(7,0) == 2) { // if tcp.session_id == 2
           // do something for session #2
           std::cout << "[TCP_CONSUMER] Received data for session id #2 ! Triggering collection #1024" << std::endl;

           // -- Example action code : please remove this in production unless you
           // really want it !
           // as example, we trigger collection #1
           enyx::oe::hwstrat::trigger_collection(output, 1024);

        }
    }
}

}}}
