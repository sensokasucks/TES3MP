#ifndef OPENMW_FLIGHTRECORDER_HPP
#define OPENMW_FLIGHTRECORDER_HPP

#include <cstdint>
#include <string>

namespace RakNet
{
    struct RakNetGUID;
    struct Packet;
    class BitStream;
}

// Packet flight recorder for desync hunting, built on the co-op flight recorder from
// github.com/cranebeaker/coop-anything (components/openmw-mp/Flight, MIT licensed).
//
// Off unless the TES3MP_FLIGHT_DIR environment variable names a folder. Then every packet this process sends
// or receives is kept in a 16 MB ring in memory and written to <folder>/<label>-<reason>-<time>.cofr on
// disconnect and on exit. Compare the files of two or more machines with
//     python -I tools/coop-anything/flight/flight.py diverge server.cofr client.cofr
// TES3MP_FLIGHT_SLOT sets this machine's slot number in the files (default: 0 for the server, 1 for a client).
namespace mwmp::Flight
{
    void init(const std::string& label, bool isServer);
    bool enabled();

    void netOut(const RakNet::RakNetGUID& to, bool broadcast, const RakNet::BitStream& stream, uint8_t channel);
    void netIn(const RakNet::Packet& packet);
    void marker(const std::string& text);

    // Writes the ring to a file now. Returns its path, or "" when recording is off or the write failed.
    std::string dump(const char* reason);
}

#endif // OPENMW_FLIGHTRECORDER_HPP
