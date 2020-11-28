//Auto generated OPS-code. DO NOT MODIFY!

package __packageName;

import ops.OPSObject;
import configlib.ArchiverInOut;
import configlib.SerializableFactory;
import configlib.Serializable;
import java.io.IOException;

public class __className extends __baseClassName
{
    public byte __className_version = __className_idlVersion;

__declarations

    private static SerializableFactory factory = new TypeFactory();

    public static String getTypeName(){return "__packageName.__className";}

    public static SerializableFactory getTypeFactory()
    {
        return factory;
    }

    public __className()
    {
        super();
        appendType(getTypeName());
__constructorBody
    }
    public void serialize(ArchiverInOut archive) throws IOException
    {
        super.serialize(archive);
__serialize
    }
    @Override
    public Object clone()
    {
        __className cloneResult = new __className();
        fillClone(cloneResult);
        return cloneResult;
    }

    @Override
    public void fillClone(OPSObject cloneO)
    {
        super.fillClone(cloneO);
        __className cloneResult = (__className)cloneO;
__cloneBody
    }

    private static class TypeFactory implements SerializableFactory
    {
        public Serializable create(String type)
        {
            if (type.equals(__className.getTypeName()))
            {
                return new __className();
            }
            else
            {
                return null;
            }
        }
    }
}
