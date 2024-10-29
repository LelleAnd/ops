/**
 * Copyright (C) 2024 Lennart Andersson.
 *
 * This file is part of OPS (Open Publish Subscribe).
 *
 * OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
 */
package ops;

/**
 *
 */
public class UdpReceiveDataHandler extends ReceiveDataHandler
{

    private boolean commonReceiver = false;

    public UdpReceiveDataHandler(Topic t, Participant part, boolean commonReceiver)
    {
        super(t, part);
        String localIf = NetworkSupport.DoSubnetTranslation(t.getLocalInterface());
        Receiver rec = ReceiverFactory.createReceiver(t, localIf);

        channels.add(new ReceiveDataChannel(t, part, rec, this));

        this.commonReceiver = commonReceiver;
        if (commonReceiver) {
            part.setUdpTransportInfo(((UDPReceiver)rec).getIP(), ((UDPReceiver)rec).getPort());
        }
    }

    @Override
    public void cleanup()
    {
        if (commonReceiver)
        {
            participant.setUdpTransportInfo("", 0);
        }
    }

}
