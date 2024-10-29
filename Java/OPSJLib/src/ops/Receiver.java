/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
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

import java.io.IOException;

/**
 *
 * @author Anton Gravestam
 */
public interface Receiver
{

    void Open() throws IOException;

    void Close();

    String getRemoteIp();
    int getRemotePort();

    //public boolean receive(byte[] bytes, int offset);
    public boolean receive(byte[] headerBytes, byte[] bytes, int offset);
    //void setReceiveTimeout(int millis) throws CommException;
    //void receive(byte[] b) throws ReceiveTimedOutException;
    //boolean receive(byte[] b);
    //void send(byte[] b);

    Event getNewBytesEvent();

}
