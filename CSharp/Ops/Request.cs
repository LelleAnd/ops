///////////////////////////////////////////////////////////
//  Request.cs
//  Implementation of the Class Request
//  Created on:      12-nov-2011 09:25:35
//  Author:
///////////////////////////////////////////////////////////

namespace Ops 
{
	public class Request : OPSObject 
    {
        private byte _Request_version = Request_idlVersion;
        public byte Request_version
        {
            get { return _Request_version; }
            set {
                if (value > Request_idlVersion)
                {
                    throw new IdlVersionException(
                        "Request: received version '" + value + "' > known version '" + Request_idlVersion + "'");
                }
                _Request_version = value;
            }
        }

        public const byte Request_idlVersion = 0;
        public string requestId = "";

		public override void Serialize(IArchiverInOut archive)
        {
            base.Serialize(archive);

            if (IdlVersionMask != 0)
            {
                Request_version = archive.Inout("Request_version", Request_version);
            }
            else
            {
                Request_version = 0;
            }

            requestId = archive.Inout("requestId", requestId);
        }

	}

}