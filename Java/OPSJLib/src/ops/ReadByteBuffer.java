/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2024 Lennart Andersson
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
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.List;
import java.util.Vector;

/**
 *
 * @author angr
 */
public class ReadByteBuffer
{
    final static byte protocolVersionLow  = 5;
    final static byte protocolVersionHigh  = 0;
    final static String protocolID = "opsp";

    //private DataInputStream dis;

    ByteBuffer inBuffer;


    /** Creates a new instance of ReadByteBuffer */
//    public ReadByteBuffer(DataInputStream dis)
//    {
//        this.setDis(dis);
//    }
    public ReadByteBuffer(byte[] buf)
    {
        inBuffer = ByteBuffer.wrap(buf);
        inBuffer.order(ByteOrder.LITTLE_ENDIAN);
    }
    public ReadByteBuffer(byte[] buf, int offset, int length)
    {
        inBuffer = ByteBuffer.wrap(buf, offset, length);
        inBuffer.order(ByteOrder.LITTLE_ENDIAN);
    }
    @Deprecated
    public static byte[] trimSegments(byte[] bytes, int segmentSize, int nrOfSegments, int headerSize, byte[] result)
    {
        //int nrOfSegments = bytes.length / segmentSize;
        //if(bytes.length % segmentSize != 0) nrOfSegments++;
        //byte[] result = new byte[bytes.length - nrOfSegments*headerSize];


        for (int i = 0; i < nrOfSegments; i++)
        {
            System.arraycopy(bytes, i*segmentSize + headerSize, result, i*(segmentSize-headerSize), segmentSize - headerSize);
        }
        return result;

    }

    public final int position()
    {
        return inBuffer.position();
    }


//    public OPSObject readOPSObject(OPSObjectHelper oh) throws IOException
//    {
//        int size = getDis().readInt();
//        byte[] bytes = new byte[size];
//        getDis().read(bytes);
//        return oh.deserialize(bytes);
//    }
//    public void readOPSObjectArr(Vector vec, OPSObjectHelper oh) throws IOException
//    {
//        int size = getDis().readInt();
//        for (int i = 0; i <size; i++)
//        {
//            vec.add(readOPSObject(oh));
//
//        }
//    }
    public boolean readboolean() throws IOException
    {
        return inBuffer.get() > 0;
    }
    public List<Boolean> readbooleanArr() throws IOException
    {
        int size = readint();
        Vector<Boolean> ret = new Vector<Boolean>(size);
        for (int i = 0; i < size ; i++)
        {
            ret.add(Boolean.valueOf(readboolean()));
        }
        return ret;
    }
    public byte readbyte() throws IOException
    {
        return inBuffer.get();
    }
    public List<Byte> readbyteArr() throws IOException
    {
        int size = readint();
        Vector<Byte> ret = new Vector<Byte>(size);

        for (int i = 0; i < size ; i++)
        {
            ret.add(Byte.valueOf(readbyte()));
        }
        return ret;
    }

    public short readshort() throws IOException
    {
        return inBuffer.getShort();
    }
    public List<Short> readshortArr() throws IOException
    {
        int size = readint();
        Vector<Short> ret = new Vector<Short>(size);
        for (int i = 0; i < size ; i++)
        {
            ret.add(Short.valueOf(readshort()));
        }
        return ret;
    }

    public void readBytes(byte[] bytes)
    {
        inBuffer.get(bytes);
    }
    public int readint() throws IOException
    {
        return inBuffer.getInt();
    }
    public List<Integer> readintArr() throws IOException
    {
        int size = readint();
        Vector<Integer> ret = new Vector<Integer>(size);
        for (int i = 0; i < size ; i++)
        {
            ret.add(Integer.valueOf(readint()));
        }
        return ret;
    }
    public long readlong() throws IOException
    {
        return inBuffer.getLong();
    }
    public List<Long> readlongArr() throws IOException
    {
        int size = readint();
        Vector<Long> ret = new Vector<Long>(size);
        for (int i = 0; i < size ; i++)
        {
            ret.add(Long.valueOf(readlong()));
        }
        return ret;
    }
    public float readfloat() throws IOException
    {
        return inBuffer.getFloat();
    }
    public List<Float> readfloatArr() throws IOException
    {
        int size = readint();
        Vector<Float> ret = new Vector<Float>(size);
        for (int i = 0; i < size ; i++)
        {
            ret.add(Float.valueOf(readfloat()));
        }
        return ret;
    }
    public double readdouble() throws IOException
    {
        return inBuffer.getDouble();
    }
    public List<Double> readdoubleArr() throws IOException
    {
        int size = readint();
        Vector<Double> ret = new Vector<Double>(size);
        for (int i = 0; i < size ; i++)
        {
            ret.add(Double.valueOf(readdouble()));
        }
        return ret;
    }
    public String readstring() throws IOException
    {
        int size = readint();
        byte[] bytes = new byte[size];
        inBuffer.get(bytes);
        return new String(bytes);
    }
    public List<String> readstringArr() throws IOException
    {
        int size = readint();
        Vector<String> ret = new Vector<String>(size);
        for (int i = 0; i < size ; i++)
        {
            ret.add(readstring());
        }
        return ret;
    }

//    public DataInputStream getDis()
//    {
//        return dis;
//    }
//
//    public void setDis(DataInputStream dis)
//    {
//        this.dis = dis;
//    }

    boolean checkProtocol() throws IOException
    {
        byte[] prtIDBytes = new byte[4];
        inBuffer.get(prtIDBytes);
        String prtID = new String(prtIDBytes);
        if(prtID.equals(protocolID))
        {
            byte verLow = readbyte();
            byte verHigh = readbyte();
            if(verHigh == protocolVersionHigh && verLow == protocolVersionLow)
            {
                return true;
            }
            return false;
        }
        return false;

    }

}
