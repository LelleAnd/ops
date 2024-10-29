using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;  // Needed for the "MethodImpl" synchronization attribute
using System.Linq;
using System.Text;

namespace Ops
{
    class TcpSendDataHandler : SendDataHandler
    {
        private readonly string sinkIP;

        public TcpSendDataHandler(Topic t, string localInterface)
        {
            sender = new TcpServerSender(t.GetDomainAddress(), t.GetPort(), t.GetOutSocketBufferSize());
            sinkIP = t.GetDomainAddress();
        }

        public override void UpdateTransportInfo(Topic top)
        {
            string ip = "";
            int port = 0;
            GetLocalEndpoint(ref ip, ref port);

            if (!Ops.InetAddress.IsValidNodeAddress(top.GetDomainAddress()))
            {
                top.SetDomainAddress(Ops.InetAddress.DoSubnetTranslation(top.GetLocalInterface()));
            }
            top.SetPort(port);
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public override bool SendData(byte[] bytes, int size, Topic t)
        {
            ///TODO How to handle one slow receiver when others are fast
            /// A separate thread for each connection and buffers ??

            return SendData(bytes, size, sinkIP, t.GetPort());
        }

    }

}
