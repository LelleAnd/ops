///////////////////////////////////////////////////////////
//  OPSObjectFactory.cs
//  Implementation of the Class OPSObjectFactory
//  Created on:      12-nov-2011 09:25:33
//  Author:
///////////////////////////////////////////////////////////

namespace Ops
{

    public class OPSSerializableFactory : ISerializableFactory
    {
        /// 
        /// <param name="typeString"></param>
        public ISerializable Create(string type)
        {
            if (type.Equals("ops.protocol.OPSMessage"))
            {
                return new OPSMessage();
            }
            if (type.Equals("ops.ParticipantInfoData"))
            {
                return new ParticipantInfoData();
            }
            if (type.Equals("opsidls.SendAckPatternData"))
            {
                return new opsidls.SendAckPatternData();
            }
            if (type.Equals("DefaultOPSConfigImpl"))
            {
                return new DefaultOPSConfigImpl();
            }
            if (type.Equals("Domain"))
            {
                return new Domain();
            }

            // Usage of "MulticastDomain" is deprecated. We must keep this for backward compatibility though.
            if (type.Equals("MulticastDomain"))
            {
                return new Domain();
            }

            if (type.Equals("Topic"))
            {
                return new Topic();
            }
            if (type.Equals("Channel"))
            {
                return new Channel();
            }
            if (type.Equals("Transport"))
            {
                return new Transport();
            }
            return null;
        }

        public string Create(object obj)
        {
            if (obj is OPSMessage)
            {
                return "ops.protocol.OPSMessage";
            }
            if (obj is DefaultOPSConfigImpl)
            {
                return "DefaultOPSConfigImpl";
            }
            if (obj is Domain)
            {
                return "Domain";
            }
            //if (obj is MulticastDomain)
            //{
            //    return "Domain";
            //}
            if (obj is Topic)
            {
                return "Topic";
            }
            if (obj is ParticipantInfoData)
            {
                return "ops.ParticipantInfoData";
            }
            return null;
        }
    }

	public class OPSObjectFactory : SerializableCompositeFactory 
    {
		internal static OPSObjectFactory instance = null;

        /// <summary>
        /// 
        /// </summary>
        public OPSObjectFactory()
        {
            Add(new OPSSerializableFactory());
        }

		/// <summary>
		/// Tries to construct the most specialized version of the given typeString
		/// </summary>
		/// <param name="typeString"></param>
		public override ISerializable Create(string typeString)
        {
            string[] types = typeString.Split(' ');
            foreach (string type in types)
            {
                ISerializable serializable = base.Create(type);
                if (serializable != null)
                {
                    return serializable;
                }
            }
            return null;
		}

		/// <summary>
		/// Create singelton instance of OPSObjectFactory.
		/// </summary>
		public static OPSObjectFactory GetInstance()
        {
            if (instance == null)
            {
                instance = new OPSObjectFactory();
            }
            return instance;
		}

	}

}