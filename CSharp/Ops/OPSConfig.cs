///////////////////////////////////////////////////////////
//  OPSConfig.cs
//  Implementation of the Class OPSConfig
//  Created on:      12-nov-2011 09:25:33
//  Author:
///////////////////////////////////////////////////////////

using System.Collections.Generic;       // Needed for the "List"
using System.IO;
using System.Text;

namespace Ops 
{
	public class OPSConfig : OPSObject 
    {
        public const byte OPSConfig_idlVersion = 0;
        private byte _OPSConfig_version = OPSConfig_idlVersion;
        public byte OPSConfig_version
        {
            get { return _OPSConfig_version; }
            set {
                if (value > OPSConfig_idlVersion)
                {
                    throw new IdlVersionException(
                        "OPSConfig: received version '" + value + "' > known version '" + OPSConfig_idlVersion + "'");
                }
                _OPSConfig_version = value;
            }
        }

        private protected List<Domain> domains = new List<Domain>();

        public static OPSConfig GetConfig() 
        {
            return OPSConfigRepository.GetConfig(); //GetConfig("ops_config.xml");
        }

        public static OPSConfig GetConfig(string configFile)
        {
            try
            {
                FileStream fis = File.OpenRead(configFile);
                XMLArchiverIn archiverIn = new XMLArchiverIn(fis, "root");
                archiverIn.Add(OPSObjectFactory.GetInstance());
                OPSConfig newConfig = null;
                newConfig = (OPSConfig)archiverIn.Inout("ops_config", newConfig);
                fis.Close();
                return newConfig;
            }
            catch (FileNotFoundException)
            {
                Logger.ExceptionLogger.LogMessage("OPSConfig::GetConfig(), File NOT found: " + configFile);
                return null;
            }
        }

        public static void SaveConfig(OPSConfig conf, string configFile) 
        {
            FileStream fos = File.OpenWrite(configFile);
            BinaryWriter bw = new BinaryWriter(fos);
            XMLArchiverOut archiverOut = new XMLArchiverOut(bw, false);
            bw.Write(Encoding.ASCII.GetBytes("<root xmlns=ops>\n"));
            archiverOut.Inout("ops_config", conf);
            bw.Write(Encoding.ASCII.GetBytes("</root>"));
            fos.Close();
        }

        //---------------------------------------------------------------

        public OPSConfig()
        {
            AppendType("OPSConfig");
        }

        public Domain GetDomain(string domainID)
        {
            foreach (Domain domain in domains)
            {
                if (domain.GetDomainID().Equals(domainID))
                {
                    return domain;
                }
            }
            return null;
        }

        public override void Serialize(IArchiverInOut archive)
        {
            base.Serialize(archive);
            if (IdlVersionMask != 0)
            {
                OPSConfig_version = archive.Inout("OPSConfig_version", OPSConfig_version);
            }
            else
            {
                OPSConfig_version = 0;
            }
            domains = (List<Domain>)archive.InoutSerializableList("domains", domains);
        }

        public List<Domain> getDomains()
        {
            return domains;
        }

        public override object Clone()
        {
            OPSConfig cloneResult = new OPSConfig();
            FillClone(cloneResult);
            return cloneResult;
        }

        public override void FillClone(OPSObject cloneO)
        {
            base.FillClone(cloneO);
            OPSConfig cloneResult = (OPSConfig)cloneO;
            cloneResult.OPSConfig_version = OPSConfig_version;
            cloneResult.domains = new List<Domain>(domains.Count);
            domains.ForEach((item) => { cloneResult.domains.Add((Domain)item.Clone()); });
        }
    }

}