//Auto generated OPS-code. DO NOT MODIFY!

package testall;

import ops.OPSObject;
import configlib.ArchiverInOut;
import java.io.IOException;

public class ChildData extends BaseData
{
	public boolean bo;
	public byte b;
	public int i;
	public long l;
	public float f;
	public double d;
	////*This string shall hold the word World*/
	public String s = "";
	public TestData test2 = new TestData();
	public java.util.Vector<Boolean> bos = new java.util.Vector<Boolean>();
	public java.util.Vector<Byte> bs = new java.util.Vector<Byte>();
	public java.util.Vector<Integer> is = new java.util.Vector<Integer>();
	public java.util.Vector<Long> ls = new java.util.Vector<Long>();
	public java.util.Vector<Float> fs = new java.util.Vector<Float>();
	public java.util.Vector<Double> ds = new java.util.Vector<Double>();
	public java.util.Vector<String> ss = new java.util.Vector<String>();
	public java.util.Vector<TestData> test2s = new java.util.Vector<TestData>();
	public java.util.Vector<TestData> test2s2 = new java.util.Vector<TestData>();


    public ChildData()
    {
        super();
        appendType("testall.ChildData");

    }
    public void serialize(ArchiverInOut archive) throws IOException
    {
        super.serialize(archive);
		bo = archive.inout("bo", bo);
		b = archive.inout("b", b);
		i = archive.inout("i", i);
		l = archive.inout("l", l);
		f = archive.inout("f", f);
		d = archive.inout("d", d);
		s = archive.inout("s", s);
		test2 = (TestData) archive.inout("test2", test2);
		bos = (java.util.Vector<Boolean>) archive.inoutBooleanList("bos", bos);
		bs = (java.util.Vector<Byte>) archive.inoutByteList("bs", bs);
		is = (java.util.Vector<Integer>) archive.inoutIntegerList("is", is);
		ls = (java.util.Vector<Long>) archive.inoutLongList("ls", ls);
		fs = (java.util.Vector<Float>) archive.inoutFloatList("fs", fs);
		ds = (java.util.Vector<Double>) archive.inoutDoubleList("ds", ds);
		ss = (java.util.Vector<String>) archive.inoutStringList("ss", ss);
		test2s = (java.util.Vector<TestData>) archive.inoutSerializableList("test2s", test2s);
		test2s2 = (java.util.Vector<TestData>) archive.inoutSerializableList("test2s2", test2s2);

    }
}