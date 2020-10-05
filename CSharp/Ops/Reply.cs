///////////////////////////////////////////////////////////
//  Reply.cs
//  Implementation of the Class Reply
//  Created on:      12-nov-2011 09:25:35
//  Author:
///////////////////////////////////////////////////////////

namespace Ops 
{
	public class Reply : OPSObject 
    {
        private byte _Reply_version = Reply_idlVersion;
        public byte Reply_version
        {
            get { return _Reply_version; }
            set {
                if (value > Reply_idlVersion)
                {
                    throw new IdlVersionException(
                        "Reply: received version '" + value + "' > known version '" + Reply_idlVersion + "'");
                }
                _Reply_version = value;
            }
        }

        public const byte Reply_idlVersion = 0;
        public string message = "";
		public bool requestAccepted = false;
		public string requestId = "";

        public override void Serialize(IArchiverInOut archive)
        {
            base.Serialize(archive);

            if (IdlVersionMask != 0)
            {
                Reply_version = archive.Inout("Reply_version", Reply_version);
            }
            else
            {
                Reply_version = 0;
            }

            requestId = archive.Inout("requestId", requestId);
            requestAccepted = archive.Inout("requestAccepted", requestAccepted);
            message = archive.Inout("message", message);
		}

	}

}