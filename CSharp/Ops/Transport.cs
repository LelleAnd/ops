///////////////////////////////////////////////////////////
//  Transport.cs
//  Implementation of the Class Channel
//  Created on:      18-oct-2016
//  Author:
///////////////////////////////////////////////////////////

using System.Collections.Generic;

namespace Ops
{
    public class Transport : OPSObject 
    {
        public const byte Transport_idlVersion = 0;
        private byte _Transport_version = Transport_idlVersion;
        public byte Transport_version
        {
            get { return _Transport_version; }
            set {
                if (value > Transport_idlVersion)
                {
                    throw new IdlVersionException(
                        "Transport: received version '" + value + "' > known version '" + Transport_idlVersion + "'");
                }
                _Transport_version = value;
            }
        }

        public string channelID = "";
        public List<string> topics = new List<string>();

        public Transport()
        {
            AppendType("Transport");
        }

        public override void Serialize(IArchiverInOut archive)
        {
            // NOTE. Keep this in sync with the C++ version, so it in theory is possible to send these as objects.
            // We need to serialize fields in the same order as C++.
            //OPSObject::serialize(archiver);
            base.Serialize(archive);

            if (IdlVersionMask != 0)
            {
                Transport_version = archive.Inout("Transport_version", Transport_version);
            }
            else
            {
                Transport_version = 0;
            }

            //archiver->inout(std::string("channelID"), channelID);
            channelID = archive.Inout("channelID", channelID);

            //archiver->inout(std::string("topics"), topics);
            topics = (List<string>)archive.InoutStringList("topics", topics);
        }

        public override object Clone()
        {
            Transport cloneResult = new Transport();
            FillClone(cloneResult);
            return cloneResult;
        }

        public override void FillClone(OPSObject cloneO)
        {
            base.FillClone(cloneO);
            Transport cloneResult = (Transport)cloneO;
            cloneResult.Transport_version = Transport_version;
            cloneResult.channelID = channelID;
            cloneResult.topics = new List<string>(topics);
        }
    }
}
