#ifndef OPENMW_BASEPACKET_HPP
#define OPENMW_BASEPACKET_HPP

#include <string>
#include <variant>
#include <components/esm/refid.hpp>
#include <components/esm/attr.hpp>
#include <components/esm3/loadskil.hpp>
#include <RakNetTypes.h>
#include <BitStream.h>
#include <PacketPriority.h>


namespace mwmp
{
    class BasePacket
    {
    public:
        explicit BasePacket(RakNet::RakPeerInterface *peer);

        virtual ~BasePacket() = default;

        virtual void Packet(RakNet::BitStream *newBitstream, bool send);
        virtual uint32_t Send(bool toOtherPlayers = true);
        virtual uint32_t Send(RakNet::AddressOrGUID destination);
        virtual void Read();

        void setGUID(RakNet::RakNetGUID newGuid);
        RakNet::RakNetGUID getGUID();

        void SetReadStream(RakNet::BitStream *bitStream);
        void SetSendStream(RakNet::BitStream *bitStream);
        void SetStreams(RakNet::BitStream *inStream, RakNet::BitStream *outStream);
        virtual uint32_t RequestData(RakNet::RakNetGUID targetGuid);

        static inline uint32_t headerSize()
        {
            return static_cast<uint32_t>(1 + RakNet::RakNetGUID::size()); // packetID + RakNetGUID (uint64_t)
        }

        uint8_t GetPacketID() const
        {
            return packetID;
        }

        bool isPacketValid() const
        {
            return packetValid;
        }

    protected:
        template<class templateType>
        bool RW(templateType &data, uint32_t size, bool write)
        {
            if (write)
                bs->Write(data, size);
            else
                return bs->Read(data, size);
            return true;
        }

        template<class templateType>
        bool RW(templateType &data, bool write, bool compress = 0)
        {
            if (write)
            {
                if (compress)
                    bs->WriteCompressed(data);
                else
                    bs->Write(data);
                return true;
            }
            else
            {
                if (compress)
                    return bs->ReadCompressed(data);
                else
                    return bs->Read(data);
            }
        }

        bool RW(bool &data, bool write)
        {
            if (write)
                bs->Write(data);
            else
                return bs->Read(data);
            return true;
        }

        const static uint32_t maxStrSize = 64 * 1024; // 64 KiB

        bool RW(std::string &str, bool write, bool compress = false, std::string::size_type maxSize = maxStrSize)
        {
            bool res = true;
            if (write)
            {
                if (compress)
                    RakNet::RakString::SerializeCompressed(str.substr(0, maxSize).c_str(), bs); // todo: remove extra copy of string
                else
                {
                    RakNet::RakString rstr;
                    rstr.AppendBytes(str.c_str(), str.size() > maxSize ? maxSize : str.size());
                    bs->Write(rstr);
                }
            }
            else
            {
                RakNet::RakString rstr;
                if (compress)
                    res = rstr.DeserializeCompressed(bs);
                else
                    res = bs->Read(rstr);

                if (res)
                {
                    rstr.Truncate(rstr.GetLength() > maxSize ? maxSize : rstr.GetLength());
                    str = rstr.C_String();
                }
                else
                    str = std::string();
            }
            return res;
        }

        bool RW(ESM::RefId &refId, bool write, bool compress = false)
        {
            std::string str;
            if (write)
                str = refId.serializeText();
            bool res = RW(str, write, compress);
            if (!write)
                refId = res ? ESM::RefId::deserializeText(str) : ESM::RefId();
            return res;
        }

        // Active effect args are either an actor id (int) or a skill/attribute RefId; on the wire
        // they are sent as a tag plus an index so the server-side Lua API can keep using ints
        bool RW(std::variant<ESM::RefId, int> &arg, bool write)
        {
            uint8_t tag = 0;
            int value = -1;
            if (write)
            {
                if (const int* actorId = std::get_if<int>(&arg))
                    value = *actorId;
                else
                {
                    const ESM::RefId& id = std::get<ESM::RefId>(arg);
                    int index = ESM::Attribute::refIdToIndex(id);
                    if (index != -1)
                        tag = 1;
                    else
                    {
                        index = ESM::Skill::refIdToIndex(id);
                        if (index != -1)
                            tag = 2;
                    }
                    value = index;
                }
            }
            bool res = RW(tag, write) && RW(value, write);
            if (!write)
            {
                if (tag == 1)
                    arg = ESM::Attribute::indexToRefId(value);
                else if (tag == 2)
                    arg = ESM::Skill::indexToRefId(value);
                else
                    arg = value;
            }
            return res;
        }

    protected:
        uint8_t packetID;
        PacketReliability reliability;
        PacketPriority priority;
        int8_t orderChannel;
        RakNet::BitStream *bsRead, *bsSend, *bs;
        RakNet::RakPeerInterface *peer;
        RakNet::RakNetGUID guid;
        bool packetValid;
    };
}

#endif //OPENMW_BASEPACKET_HPP
