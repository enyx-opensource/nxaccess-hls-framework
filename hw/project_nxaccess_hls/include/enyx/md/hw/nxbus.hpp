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

#include <cstddef>
#include <stdint.h>
#include <ap_int.h>

namespace enyx {
namespace md {
namespace hw {

struct nxbus_meta_sizes {
    // nxBus Sizes
    static std::size_t const NXBUS_SIZE_END_OF_EXTRA        = 1;
    static std::size_t const NXBUS_SIZE_MARKET_INTERNAL_ID  = 4;
    static std::size_t const NXBUS_SIZE_OPCODE              = 8;
    static std::size_t const NXBUS_SIZE_ORDER_ID            = 64;
    static std::size_t const NXBUS_SIZE_SIDE_BUYNSELL       = 1;
    static std::size_t const NXBUS_SIZE_QTY                 = 32;
    static std::size_t const NXBUS_SIZE_PRICE               = 64;
    static std::size_t const NXBUS_SIZE_TIMESTAMP           = 32;
    static std::size_t const NXBUS_SIZE_INSTR_ASCII         = 128;
    static std::size_t const NXBUS_SIZE_INSTR_BIN           = 32;
    static std::size_t const NXBUS_SIZE_INSTR_ID            = 24;
    static std::size_t const NXBUS_SIZE_DATA0               = 64;
    static std::size_t const NXBUS_SIZE_DATA1               = 32;
    static std::size_t const NXBUS_SIZE_DATA2               = 64;
    static std::size_t const NXBUS_EXTRA_DATA_MAX_SIZE      = 256;
};

// This meta struct is only here to avoid coding errors
// C++ allows offsetof() to get the offset of the BYTE-aligned sturct, representing bit-aligned FPGA struct
#pragma pack(1) // ensure C++ packs structures correclty
struct nxbus_meta_struct{

    char NXBUS_SIZE_DATA2[64];
    char NXBUS_SIZE_DATA1[32];
    char NXBUS_SIZE_DATA0[64];
    char NXBUS_SIZE_INSTR_ID[24];
    char NXBUS_SIZE_INSTR_BIN[32];
    char NXBUS_SIZE_INSTR_ASCII[128];
    char NXBUS_SIZE_TIMESTAMP[32];
    char NXBUS_SIZE_PRICE[64];
    char NXBUS_SIZE_QTY[32];
    char NXBUS_SIZE_SIDE_BUYNSELL[1];
    char NXBUS_SIZE_ORDER_ID[64];
    char NXBUS_SIZE_OPCODE[8];
    char NXBUS_SIZE_MARKET_INTERNAL_ID[4];
    char NXBUS_SIZE_END_OF_EXTRA[nxbus_meta_sizes::NXBUS_SIZE_END_OF_EXTRA];

    char NXBUS_EXTRA_DATA_MAX_SIZE[256];
};

struct nxbus_axi {

    static std::size_t const data_width = 550;
    ap_uint<data_width> data;

    static std::size_t const user_width = 16;
    ap_uint<user_width> user;

    ap_uint<1> last;
};


#define ARG_TO_MEMBER(__member, __struct_meta_member) __member = \
    arg.data(offsetof(nxbus_meta_struct, __struct_meta_member), \
             offsetof(nxbus_meta_struct, __struct_meta_member) + nxbus_meta_sizes::__struct_meta_member -1)

#define MEMBER_TO_ARG(__member, __struct_meta_member) \
    arg.data(offsetof(nxbus_meta_struct, __struct_meta_member), \
             offsetof(nxbus_meta_struct, __struct_meta_member) + nxbus_meta_sizes::__struct_meta_member -1) = __member


struct nxbus {

    nxbus() {} // default

    /// Conversion operator from AXI bus
    nxbus(nxbus_axi const arg) {
        ARG_TO_MEMBER(end_of_extra, NXBUS_SIZE_END_OF_EXTRA) ;
        ARG_TO_MEMBER(market_internal_id, NXBUS_SIZE_MARKET_INTERNAL_ID) ;
        ARG_TO_MEMBER(opcode, NXBUS_SIZE_OPCODE) ;
        ARG_TO_MEMBER(order_id, NXBUS_SIZE_ORDER_ID) ;
        ARG_TO_MEMBER(buy_nsell, NXBUS_SIZE_SIDE_BUYNSELL) ;
        ARG_TO_MEMBER(qty, NXBUS_SIZE_QTY) ;
        ARG_TO_MEMBER(price, NXBUS_SIZE_PRICE) ;
        ARG_TO_MEMBER(timestamp, NXBUS_SIZE_TIMESTAMP) ;
        ARG_TO_MEMBER(instr_ascii, NXBUS_SIZE_INSTR_ASCII) ;
        ARG_TO_MEMBER(instr_bin, NXBUS_SIZE_INSTR_BIN) ;
        ARG_TO_MEMBER(instr_id, NXBUS_SIZE_INSTR_ID) ;
        ARG_TO_MEMBER(data0, NXBUS_SIZE_DATA0) ;
        ARG_TO_MEMBER(data1, NXBUS_SIZE_DATA1) ;
        ARG_TO_MEMBER(data2, NXBUS_SIZE_DATA2) ;

    }

    /// Conversion operator to AXI bus
    operator nxbus_axi() {
        nxbus_axi arg;
        MEMBER_TO_ARG(end_of_extra, NXBUS_SIZE_END_OF_EXTRA) ;
        MEMBER_TO_ARG(market_internal_id, NXBUS_SIZE_MARKET_INTERNAL_ID) ;
        MEMBER_TO_ARG(opcode, NXBUS_SIZE_OPCODE) ;
        MEMBER_TO_ARG(order_id, NXBUS_SIZE_ORDER_ID) ;
        MEMBER_TO_ARG(buy_nsell, NXBUS_SIZE_SIDE_BUYNSELL) ;
        MEMBER_TO_ARG(qty, NXBUS_SIZE_QTY) ;
        MEMBER_TO_ARG(price, NXBUS_SIZE_PRICE) ;
        MEMBER_TO_ARG(timestamp, NXBUS_SIZE_TIMESTAMP) ;
        MEMBER_TO_ARG(instr_ascii, NXBUS_SIZE_INSTR_ASCII) ;
        MEMBER_TO_ARG(instr_bin, NXBUS_SIZE_INSTR_BIN) ;
        MEMBER_TO_ARG(instr_id, NXBUS_SIZE_INSTR_ID) ;
        MEMBER_TO_ARG(data0, NXBUS_SIZE_DATA0) ;
        MEMBER_TO_ARG(data1, NXBUS_SIZE_DATA1) ;
        MEMBER_TO_ARG(data2, NXBUS_SIZE_DATA2) ;


        return arg;
    }

    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_END_OF_EXTRA> end_of_extra;
    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_MARKET_INTERNAL_ID> market_internal_id;
    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_OPCODE> opcode;
    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_ORDER_ID> order_id;
    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_SIDE_BUYNSELL> buy_nsell;
    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_QTY> qty;
    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_PRICE> price;
    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_TIMESTAMP> timestamp;
    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_INSTR_ASCII> instr_ascii;
    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_INSTR_BIN> instr_bin;
    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_INSTR_ID> instr_id;
    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_DATA0> data0;
    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_DATA1> data1;
    ap_uint<nxbus_meta_sizes::NXBUS_SIZE_DATA2> data2;
};

#undef ARG_TO_MEMBER
#undef MEMBER_TO_ARG

// Instrument ID Constants
static uint32_t NXBUS_NOT_FOUND_INSTRUMENT_INTERNAL_ID = 0x00000000;
static uint32_t NXBUS_CMD_NOT_INSTRUMENT_INTERNAL_ID   = 0xFFFFFFFF;
static uint32_t NXBUS_NOT_SET_INSTRUMENT_INTERNAL_ID   = 0xFFFFFFFE;

// Price Specific Constants
static uint64_t MARKET_ORDER_BUY_PRICE  = 0x7FFFFFFFFFFFFFFF;
static uint64_t MARKET_ORDER_SELL_PRICE = 0x7FFFFFFFFFFFFFFF;
static uint32_t NXBUS_PRICE_PRECISION   = 10;

// Opcodes
// Status
static uint8_t  NXBUS_OPCODE_STATUS_INSTR                 = 0x11;
static uint8_t  NXBUS_OPCODE_STATUS_GROUP                 = 0x12;
static uint8_t  NXBUS_OPCODE_STATUS_MARKET                = 0x13;

// by order
static uint8_t NXBUS_OPCODE_ORDER_ADD                     = 0x21;
static uint8_t NXBUS_OPCODE_ORDER_EXEC                    = 0x22;
static uint8_t NXBUS_OPCODE_ORDER_EXEC_PRICE              = 0x23;
static uint8_t NXBUS_OPCODE_ORDER_EXEC_PRICE_QTY          = 0x24;
static uint8_t NXBUS_OPCODE_ORDER_REDUCE                  = 0x25;
static uint8_t NXBUS_OPCODE_ORDER_MODIFY                  = 0x26;
static uint8_t NXBUS_OPCODE_ORDER_DEL                     = 0x27;
static uint8_t NXBUS_OPCODE_ORDER_REPLACE                 = 0x28;
static uint8_t NXBUS_OPCODE_ORDER_MODIFY_PRICE            = 0x29;
static uint8_t NXBUS_OPCODE_ORDER_MODIFY_QTY              = 0x2A;

// Managed order
static uint8_t NXBUS_OPCODE_MANAGED_ORDER_ADD             = 0x31;
static uint8_t NXBUS_OPCODE_MANAGED_ORDER_EXEC            = 0x32;
static uint8_t NXBUS_OPCODE_MANAGED_ORDER_EXEC_PRICE      = 0x33;
static uint8_t NXBUS_OPCODE_MANAGED_ORDER_EXEC_PRICE_QTY  = 0x34;
static uint8_t NXBUS_OPCODE_MANAGED_ORDER_REDUCE          = 0x35;
static uint8_t NXBUS_OPCODE_MANAGED_ORDER_MODIFY          = 0x36;
static uint8_t NXBUS_OPCODE_MANAGED_ORDER_DEL             = 0x37;
static uint8_t NXBUS_OPCODE_MANAGED_ORDER_REPLACE         = 0x38;
static uint8_t NXBUS_OPCODE_MANAGED_ORDER_MODIFY_PRICE    = 0x39;
static uint8_t NXBUS_OPCODE_MANAGED_ORDER_MODIFY_QTY      = 0x3A;

// by limit
static uint8_t NXBUS_OPCODE_LIMIT_ADD                     = 0x41;
static uint8_t NXBUS_OPCODE_LIMIT_CHANGE                  = 0x42;
static uint8_t NXBUS_OPCODE_LIMIT_DEL                     = 0x43;
static uint8_t NXBUS_OPCODE_LIMIT_DEL_SUP_EQUAL           = 0x44;
static uint8_t NXBUS_OPCODE_LIMIT_DEL_INF_EQUAL           = 0x45;

// by price
static uint8_t NXBUS_OPCODE_PRICE_UPDATE                  = 0x51;
static uint8_t NXBUS_OPCODE_PRICE_MANAGED_UPDATE          = 0x52;
static uint8_t NXBUS_OPCODE_PRICE_ADD_QTY                 = 0x53;
static uint8_t NXBUS_OPCODE_PRICE_REDUCE_QTY              = 0x54;

// Trade
static uint8_t NXBUS_OPCODE_TRADE_REPORT                  = 0x61;
static uint8_t NXBUS_OPCODE_TRADE_BREAK                   = 0x62;
static uint8_t NXBUS_OPCODE_TRADE_CORRECTION              = 0x63;
static uint8_t NXBUS_OPCODE_TRADE_SUMMARY                 = 0x64;
static uint8_t NXBUS_OPCODE_TRADE_SUMMARY_BREAK           = 0x65;
static uint8_t NXBUS_OPCODE_TRADE_SUMMARY_CORRECTION      = 0x66;

// Auction
static uint8_t NXBUS_OPCODE_AUCTION_POTENTIAL             = 0x71;

// System info
static uint8_t NXBUS_OPCODE_SYSTEM_INFO_ERROR             = 0x81;
static uint8_t NXBUS_OPCODE_SYSTEM_INFO_MISSING_MSG       = 0x82;
static uint8_t NXBUS_OPCODE_SYSTEM_INFO_LOST_SYNC         = 0x83;
static uint8_t NXBUS_OPCODE_SYSTEM_INFO_SYNC              = 0x84;
static uint8_t NXBUS_OPCODE_SYSTEM_INFO_SYNC_TIMEOUT      = 0x85;
static uint8_t NXBUS_OPCODE_SYSTEM_INFO_STATS             = 0x86;

// Misc
static uint8_t NXBUS_OPCODE_MISC_TIME                     = 0x91;
static uint8_t NXBUS_OPCODE_MISC_STOCK_DESCRIPTION        = 0x92;
static uint8_t NXBUS_OPCODE_MISC_BOOK_RESET               = 0x93;
static uint8_t NXBUS_OPCODE_MISC_SET_SEQNUM               = 0x94;
static uint8_t NXBUS_OPCODE_MISC_INPUT_PKT_INFO           = 0x95;
static uint8_t NXBUS_OPCODE_MISC_HEARTBEAT                = 0x96;
static uint8_t NXBUS_OPCODE_MISC_END_PKT                  = 0x97;
static uint8_t NXBUS_OPCODE_MISC_MARKET_ERROR             = 0x98;

// Extra
static uint8_t NXBUS_OPCODE_EXTRA_EMPTY                   = 0xA1;
static uint8_t NXBUS_OPCODE_EXTRA_INSTR                   = 0xA2;
static uint8_t NXBUS_OPCODE_EXTRA_GROUP                   = 0xA3;

// Book
static uint8_t NXBUS_OPCODE_BOOK_UPDATE                   = 0xC1;
static uint8_t NXBUS_OPCODE_BOOK_SNAPSHOT                 = 0xC2;

}}} // Namespaces
