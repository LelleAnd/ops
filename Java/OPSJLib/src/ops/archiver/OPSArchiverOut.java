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
import java.io.IOException;
import java.util.List;
import ops.OPSObject;
import ops.WriteByteBuffer;

/**
 *
 * @author angr
 */
public class OPSArchiverOut extends ArchiverInOut
{
    WriteByteBuffer writeBuf;
    private boolean optNonVirt = false;

    public OPSArchiverOut(WriteByteBuffer buf, boolean optNonVirt_)
    {
        this.writeBuf = buf;
        this.optNonVirt = optNonVirt_;
    }
    public byte[] getBytes()
    {
        return writeBuf.getBytes();
    }

    public boolean isOut() { return true; }

    public int inout(String name, int v) throws IOException
    {
        writeBuf.write(v);
        return v;
    }

    public long inout(String name, long v) throws IOException
    {
        writeBuf.write(v);
        return v;
    }

    public byte inout(String name, byte v) throws IOException
    {
        writeBuf.write(v);
        return v;
    }

    public short inout(String name, short v) throws IOException
    {
        writeBuf.write(v);
        return v;
    }

    public float inout(String name, float v) throws IOException
    {
        writeBuf.write(v);
        return v;
    }

    public boolean inout(String name, boolean v) throws IOException
    {
        writeBuf.write(v);
        return v;
    }

    public String inout(String name, String v) throws IOException
    {
        writeBuf.write(v);
        return v;
    }

    public double inout(String name, double v) throws IOException
    {
        writeBuf.write(v);
        return v;
    }

    public Serializable inout(String name, Serializable v) throws IOException
    {
        String types = "";
        if (((OPSObject)v).idlVersionMask != 0) {
            types = "0 ";
        }
        writeBuf.write(types + ((OPSObject)v).getTypesString());
        v.serialize(this);
        return v;
    }

    public <T extends Serializable> Serializable inout(String name, Serializable v, Class<T> cls) throws IOException
    {
        String types = "";
        if (((OPSObject)v).idlVersionMask != 0) {
            types = "0 ";
        }
        if (optNonVirt) {
            writeBuf.write(types);
        } else {
            writeBuf.write(types + ((OPSObject)v).getTypesString());
        }
        v.serialize(this);
        return v;
    }

    public List<Integer> inoutIntegerList(String name, List<Integer> v) throws IOException
    {
        writeBuf.writeintArr(v);
        return v;
    }

    public List<Long> inoutLongList(String name, List<Long> v) throws IOException
    {
        writeBuf.writelongArr(v);
        return v;
    }

    public List<Byte> inoutByteList(String name, List<Byte> v) throws IOException
    {
        writeBuf.writebyteArr(v);
        return v;
    }

    public List<Short> inoutShortList(String name, List<Short> v) throws IOException
    {
        writeBuf.writeshortArr(v);
        return v;
    }

    public List<Float> inoutFloatList(String name, List<Float> v) throws IOException
    {
        writeBuf.writefloatArr(v);
        return v;
    }

    public List<Boolean> inoutBooleanList(String name, List<Boolean> v) throws IOException
    {
        writeBuf.writebooleanArr(v);
        return v;
    }

    public List<String> inoutStringList(String name, List<String> v) throws IOException
    {
        writeBuf.writestringArr(v);
        return v;
    }

    public List<Double> inoutDoubleList(String name, List<Double> v) throws IOException
    {
        writeBuf.writedoubleArr(v);
        return v;
    }

    public List inoutSerializableList(String name, List v) throws IOException
    {
        writeBuf.write(v.size());
        for (int i = 0; i < v.size(); i++) {
            String types = "";
            if (((OPSObject)v.get(i)).idlVersionMask != 0) {
                types = "0 ";
            }
            writeBuf.write(types + ((OPSObject)v.get(i)).getTypesString());
            ((Serializable)v.get(i)).serialize(this);
        }
        return v;
    }

    public <T extends Serializable> List inoutSerializableList(String name, List v, Class<T> cls) throws IOException
    {
        writeBuf.write(v.size());
        for (int i = 0; i < v.size(); i++) {
            String types = "";
            if (((OPSObject)v.get(i)).idlVersionMask != 0) {
                types = "0 ";
            }
            if (optNonVirt) {
                writeBuf.write(types);
            } else {
                writeBuf.write(types + ((OPSObject)v.get(i)).getTypesString());
            }
            ((Serializable)v.get(i)).serialize(this);
        }
        return v;
    }

}
