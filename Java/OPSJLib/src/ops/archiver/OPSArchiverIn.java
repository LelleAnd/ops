/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2020 Lennart Andersson.
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

package ops.archiver;

import configlib.ArchiverInOut;
import configlib.Serializable;
import configlib.SerializableCompositeFactory;
import configlib.SerializableFactory;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;
import ops.ReadByteBuffer;
import ops.OPSObject;

/**
 *
 * @author angr
 */
public class OPSArchiverIn extends ArchiverInOut
{

    private SerializableCompositeFactory compositeFactory;
    private ReadByteBuffer readBuf;

    public OPSArchiverIn(ReadByteBuffer buf)
    {
        compositeFactory = OPSObjectFactory.getInstance();
        readBuf = buf;
    }

    public boolean isOut() { return false; }

    public boolean remove(Object o)
    {
        return compositeFactory.remove(o);
    }

    public boolean add(SerializableFactory e)
    {
        return compositeFactory.add(e);
    }
    public int inout(String name, int v) throws IOException
    {
        return readBuf.readint();
    }

    public long inout(String name, long v) throws IOException
    {
        return readBuf.readlong();
    }

    public byte inout(String name, byte v) throws IOException
    {
        return readBuf.readbyte();
    }

    public short inout(String name, short v) throws IOException
    {
        return readBuf.readshort();
    }

    public float inout(String name, float v) throws IOException
    {
        return readBuf.readfloat();
    }

    public boolean inout(String name, boolean v) throws IOException
    {
        return readBuf.readboolean();
    }

    public String inout(String name, String v) throws IOException
    {
        return readBuf.readstring();
    }

    public double inout(String name, double v) throws IOException
    {
        return readBuf.readdouble();
    }

    public <T extends Serializable> Serializable inout(String name, Serializable v, Class<T> cls) throws IOException
    {
        String type = readBuf.readstring();     // Skip string for non virtual objects, type is known by T
        Serializable newSer = null;
        try {
            newSer = (Serializable) cls.newInstance();
            if ((type.length() > 0) && (type.charAt(0) == '0')) {
                ((OPSObject)newSer).idlVersionMask = 1;   // Some value != 0
            }
            newSer.serialize(this);
        } catch (InstantiationException | IllegalAccessException e) {
        }
        return newSer;
    }

    public Serializable inout(String name, Serializable v) throws IOException
    {
        String type = readBuf.readstring();
        int verMask = 0;
        if ((type.length() > 0) && (type.charAt(0) == '0')) {
            verMask = 1;
            type = type.substring(2);
        }
        Serializable newSer = compositeFactory.create(type);
        if (newSer != null) {
            ((OPSObject)newSer).idlVersionMask = verMask;
            newSer.serialize(this);
        }
        return newSer;
    }

    public List<Integer> inoutIntegerList(String name, List<Integer> v) throws IOException
    {
        return (List<Integer>) readBuf.readintArr();
    }

    public List<Long> inoutLongList(String name, List<Long> v) throws IOException
    {
        return (List<Long>) readBuf.readlongArr();
    }

    public List<Byte> inoutByteList(String name, List<Byte> v) throws IOException
    {
        return (List<Byte>) readBuf.readbyteArr();
    }

    public List<Short> inoutShortList(String name, List<Short> v) throws IOException
    {
        return (List<Short>) readBuf.readshortArr();
    }

    public List<Float> inoutFloatList(String name, List<Float> v) throws IOException
    {
        return (List<Float>) readBuf.readfloatArr();
    }

    public List<Boolean> inoutBooleanList(String name, List<Boolean> v) throws IOException
    {
         return (List<Boolean>) readBuf.readbooleanArr();
    }

    public List<String> inoutStringList(String name, List<String> v) throws IOException
    {
         return (List<String>) readBuf.readstringArr();
    }

    public List<Double> inoutDoubleList(String name, List<Double> v) throws IOException
    {
         return (List<Double>) readBuf.readdoubleArr();
    }

    public List inoutSerializableList(String name, List v) throws IOException
    {
        Vector list = new Vector();
        int size = readBuf.readint();
        for (int i = 0; i < size; i++)
        {
            list.add(inout("", (Serializable)null));
        }
        return list;
    }

    public <T extends Serializable> List inoutSerializableList(String name, List v, Class<T> cls) throws IOException
    {
        Vector list = new Vector();
        int size = readBuf.readint();
        for (int i = 0; i < size; i++)
        {
            list.add(inout("", (Serializable)null, cls));
        }
        return list;
    }

}
