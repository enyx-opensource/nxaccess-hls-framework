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

#pragma once

#include <stdint.h>
#include <hls_stream.h>

#include "../include/enyx/oe/hwstrat/nxoe.hpp"
#include "../include/enyx/md/hw/nxbus.hpp"

#include "messages.hpp"
#include "../include/enyx/hfp/hls/hfp.hpp"
#include "configuration.hpp"

/// Implements the main process of the decision logic
void
algorithm_entrypoint(hls::stream<enyx::md::hw::nxbus_axi> & nxbus_in,
                     hls::stream<enyx::hfp::hls::dma_user_channel_data_in>& user_dma_channel_data_in,
                     hls::stream<enyx::hfp::hls::dma_user_channel_data_out>& user_dma_channel_data_out,
                     hls::stream<enyx::oe::hwstrat::trigger_command_axi> & trigger_bus_out,
                //     hls::stream<enyx::oe::hwstrat::tcp_replies> & tcp_replies_in,
                     uint32_t * supported_instrument_count);
