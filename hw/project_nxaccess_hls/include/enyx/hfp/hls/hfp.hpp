//--------------------------------------------------------------------------------
//--! Licensed Materials - Property of ENYX
//--!
//--! Organization:          Enyx
//--! Project Identifier:    010 - Enyx nxAccess HLS Framework
//--! Author:                Raphael Charolois (raphael.charolois@enyx.com)
//--!
//--! © Copyright            Enyx 2019
//--! © Copyright Notice:    The source code for this program is not published or otherwise divested of its trade secrets, 
//--!                        irrespective of what has been deposited with the U.S. Copyright Office.
//--------------------------------------------------------------------------------

#pragma once

#include <ap_int.h>

namespace enyx {
namespace hfp {
namespace hls {

// This currently needs to be defined here because the Vivado tools
// loses track of multiple definitions and then cannot cast
// properly. This is a limitation of the Vivado environment
// that will be fixed in future versions.

// Messages received/sent from/to DMA
#ifndef ENYX_NO_HLS_SUPPORT
struct dma_user_channel_data_in {
    ap_uint<128> data;
    ap_uint<1> last;
};

struct dma_user_channel_data_out {
    ap_uint<128> data;
    ap_uint<1> last;
};
#endif


}}} // Namespaces
