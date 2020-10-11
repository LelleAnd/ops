
```
//Auto generated OPS-code. DO NOT MODIFY!

package samples;

import ops.OPSObject;
import configlib.ArchiverInOut;
import configlib.SerializableFactory;
import configlib.Serializable;
import java.io.IOException;

public class SampleData extends OPSObject
{
    public byte SampleData_version = SampleData_idlVersion;

    public enum Order {
        UNDEFINED, START, STOP
    };

    public static final byte SampleData_idlVersion = 0;
    public static final int max = 42;

    public boolean boo;
    public byte b;
    public short sh;
    public int i;
    public long l;
    public float f;
    public double d;
    public String s = "";
    public String s25 = "";
    public UserData uData = new UserData();
    public Order command;
    public java.util.Vector<Boolean> boos = new java.util.Vector<Boolean>();
    public java.util.Vector<Byte> bytes = new java.util.Vector<Byte>();
    public java.util.Vector<Short> shorts = new java.util.Vector<Short>();
    public java.util.Vector<Integer> ints = new java.util.Vector<Integer>();
    public java.util.Vector<Long> longs = new java.util.Vector<Long>();
    public java.util.Vector<Float> floats = new java.util.Vector<Float>();
    public java.util.Vector<Double> doubles = new java.util.Vector<Double>();
    public java.util.Vector<String> strings = new java.util.Vector<String>();
    public java.util.Vector<String> s43vect = new java.util.Vector<String>();
    public java.util.Vector<UserData> uDatas = new java.util.Vector<UserData>();
    public java.util.Vector<Integer> intarr = new java.util.Vector<Integer>();


    private static SerializableFactory factory = new TypeFactory();

    public static String getTypeName(){return "samples.SampleData";}

    public static SerializableFactory getTypeFactory()
    {
        return factory;
    }

    public SampleData()
    {
        super();
        appendType(getTypeName());
        command = Order.UNDEFINED;
        for (int i = 0; i < 42; i++) intarr.add(0);

    }
    public void serialize(ArchiverInOut archive) throws IOException
    {
        super.serialize(archive);
        if (idlVersionMask != 0) {
            byte tmp = archive.inout("SampleData_version", SampleData_version);
            if (tmp > SampleData_idlVersion) {
                throw new IOException("SampleData: received version '" + tmp + "' > known version '" + SampleData_idlVersion + "'");
            }
            SampleData_version = tmp;
        } else {
            SampleData_version = 0;
        }
        boo = archive.inout("boo", boo);
        b = archive.inout("b", b);
        sh = archive.inout("sh", sh);
        i = archive.inout("i", i);
        l = archive.inout("l", l);
        f = archive.inout("f", f);
        d = archive.inout("d", d);
        s = archive.inout("s", s);
        s25 = archive.inout("s25", s25);
        uData = (UserData) archive.inout("uData", uData, UserData.class);
        command = archive.inoutEnum("command", command, Order.values());
        boos = (java.util.Vector<Boolean>) archive.inoutBooleanList("boos", boos);
        bytes = (java.util.Vector<Byte>) archive.inoutByteList("bytes", bytes);
        shorts = (java.util.Vector<Short>) archive.inoutShortList("shorts", shorts);
        ints = (java.util.Vector<Integer>) archive.inoutIntegerList("ints", ints);
        longs = (java.util.Vector<Long>) archive.inoutLongList("longs", longs);
        floats = (java.util.Vector<Float>) archive.inoutFloatList("floats", floats);
        doubles = (java.util.Vector<Double>) archive.inoutDoubleList("doubles", doubles);
        strings = (java.util.Vector<String>) archive.inoutStringList("strings", strings);
        s43vect = (java.util.Vector<String>) archive.inoutStringList("s43vect", s43vect);
        uDatas = (java.util.Vector<UserData>) archive.inoutSerializableList("uDatas", uDatas, UserData.class);
        intarr = (java.util.Vector<Integer>) archive.inoutIntegerList("intarr", intarr);

    }
    @Override
    public Object clone()
    {
        SampleData cloneResult = new SampleData();
        fillClone(cloneResult);
        return cloneResult;
    }

    @Override
    public void fillClone(OPSObject cloneO)
    {
        super.fillClone(cloneO);
        SampleData cloneResult = (SampleData)cloneO;
        cloneResult.SampleData_version = this.SampleData_version;
        cloneResult.boo = this.boo;
        cloneResult.b = this.b;
        cloneResult.sh = this.sh;
        cloneResult.i = this.i;
        cloneResult.l = this.l;
        cloneResult.f = this.f;
        cloneResult.d = this.d;
        cloneResult.s = this.s;
        cloneResult.s25 = this.s25;
        cloneResult.uData = (UserData)this.uData.clone();
        cloneResult.command = this.command;
        cloneResult.boos = (java.util.Vector)this.boos.clone();
        cloneResult.bytes = (java.util.Vector)this.bytes.clone();
        cloneResult.shorts = (java.util.Vector)this.shorts.clone();
        cloneResult.ints = (java.util.Vector)this.ints.clone();
        cloneResult.longs = (java.util.Vector)this.longs.clone();
        cloneResult.floats = (java.util.Vector)this.floats.clone();
        cloneResult.doubles = (java.util.Vector)this.doubles.clone();
        cloneResult.strings = (java.util.Vector)this.strings.clone();
        cloneResult.s43vect = (java.util.Vector)this.s43vect.clone();
        cloneResult.uDatas = new java.util.Vector<UserData>();
        this.uDatas.forEach((item) -> cloneResult.uDatas.add((UserData)item.clone()));
        cloneResult.intarr = (java.util.Vector)this.intarr.clone();

    }

    private static class TypeFactory implements SerializableFactory
    {
        public Serializable create(String type)
        {
            if (type.equals(SampleData.getTypeName()))
            {
                return new SampleData();
            }
            else
            {
                return null;
            }
        }
    }
}
```
