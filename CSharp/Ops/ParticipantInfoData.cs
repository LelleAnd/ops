///////////////////////////////////////////////////////////
//  ParticipantInfoData.cs
//  Implementation of the Class ParticipantInfoData
//  Created on:      01-jan-2013 
//  Author:
///////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.ComponentModel;

namespace Ops
{
    // NOTE. Must be kept in sync with C++
	public class ParticipantInfoData : OPSObject
	{
        private byte _ParticipantInfoData_version = ParticipantInfoData_idlVersion;
        public byte ParticipantInfoData_version
        {
            get { return _ParticipantInfoData_version; }
            set {
                if (value > ParticipantInfoData_idlVersion)
                {
                    throw new IdlVersionException(
                        "ParticipantInfoData: received version '" + value + "' > known version '" + ParticipantInfoData_idlVersion + "'");
                }
                _ParticipantInfoData_version = value;
            }
        }

        public const byte ParticipantInfoData_idlVersion = 0;
        public string name { get; set; }
        public string id { get; set; }
        public string domain { get; set; }
        public string ip { get; set; }
        public string languageImplementation { get; set; }
        public string opsVersion { get; set; }
        public int mc_udp_port { get; set; }
        public int mc_tcp_port { get; set; }

		private List<TopicInfoData> _subscribeTopics = new List<TopicInfoData>();
        public List<TopicInfoData> subscribeTopics { get { return _subscribeTopics; } set { _subscribeTopics = value; } }

        private List<TopicInfoData> _publishTopics = new List<TopicInfoData>();
        public List<TopicInfoData> publishTopics { get { return _publishTopics; } set { _publishTopics = value; } }

        // C++ version: std::vector<string> knownTypes;
        private List<string> _knownTypes = new List<string>();
        [Editor(@"System.Windows.Forms.Design.StringCollectionEditor," +
            "System.Design, Version=2.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a",
            typeof(System.Drawing.Design.UITypeEditor))]
        public List<string> knownTypes { get { return _knownTypes; } set { _knownTypes = value; } }

		
		public ParticipantInfoData()
		{
			AppendType("ops.ParticipantInfoData");
		}

        public override void Serialize(IArchiverInOut archive)
		{
			base.Serialize(archive);

            if (IdlVersionMask != 0)
            {
                ParticipantInfoData_version = archive.Inout("ParticipantInfoData_version", ParticipantInfoData_version);
            }
            else
            {
                ParticipantInfoData_version = 0;
            }

            name = archive.Inout("name", name);
			domain = archive.Inout("domain", domain);
			id = archive.Inout("id", id);
			ip = archive.Inout("ip", ip);
			languageImplementation = archive.Inout("languageImplementation", languageImplementation);
			opsVersion = archive.Inout("opsVersion", opsVersion);
			mc_udp_port = archive.Inout("mc_udp_port", mc_udp_port);
			mc_tcp_port = archive.Inout("mc_tcp_port", mc_tcp_port);
            _subscribeTopics = (List<TopicInfoData>)archive.InoutSerializableList<TopicInfoData>("subscribeTopics", _subscribeTopics);
            _publishTopics = (List<TopicInfoData>)archive.InoutSerializableList<TopicInfoData>("publishTopics", _publishTopics);
            _knownTypes = (List<string>)archive.InoutStringList("knownTypes", _knownTypes);
		}
	};

}

