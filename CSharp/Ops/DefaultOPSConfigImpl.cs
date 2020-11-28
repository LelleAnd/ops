///////////////////////////////////////////////////////////
//  DefaultOPSConfigImpl.cs
//  Implementation of the Class DefaultOPSConfigImpl
//  Created on:      12-nov-2011 09:25:29
//  Author:
///////////////////////////////////////////////////////////

namespace Ops 
{
	public class DefaultOPSConfigImpl : OPSConfig 
    {
        private byte _DefaultOPSConfigImpl_version = DefaultOPSConfigImpl_idlVersion;
        public byte DefaultOPSConfigImpl_version
        {
            get { return _DefaultOPSConfigImpl_version; }
            set
            {
                if (value > DefaultOPSConfigImpl_idlVersion)
                {
                    throw new IdlVersionException(
                        "DefaultOPSConfigImpl: received version '" + value + "' > known version '" + DefaultOPSConfigImpl_idlVersion + "'");
                }
                _DefaultOPSConfigImpl_version = value;
            }
        }

        public const byte DefaultOPSConfigImpl_idlVersion = 0;

        public DefaultOPSConfigImpl()
        {
            AppendType("DefaultOPSConfigImpl");
        }

        public override void Serialize(IArchiverInOut archive) 
        {
            base.Serialize(archive);
            if (IdlVersionMask != 0)
            {
                DefaultOPSConfigImpl_version = archive.Inout("DefaultOPSConfigImpl_version", DefaultOPSConfigImpl_version);
            }
            else
            {
                DefaultOPSConfigImpl_version = 0;
            }
        }
    }

}