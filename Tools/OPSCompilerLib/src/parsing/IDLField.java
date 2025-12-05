/*
 * IDLField.java
 *
 * Created on den 12 november 2007, 08:09
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package parsing;

/**
 *
 * @author angr
 */

import java.util.ArrayList;
import java.util.List;

public class IDLField
{
    private String name;
    private String type;
    private String comment;
    private List<String> directives = new ArrayList<>();
    private String value;
    private boolean idlType;
    private boolean enumType;
    private boolean intType;
    private boolean floatType;
    private boolean stringType;
    private boolean booleanType;
    private boolean _static;
    private boolean array;
    private boolean _abstract;
    private long arraySize = 0;
    private long stringSize = 0;
    private String fullyQualifiedType;
    private String rangeLo = "";
    private String rangeHi = "";
    private long arrayMaxSize = 0;
    private long stringMaxSize = 0;

    /** Creates a new instance of IDLField */
    public IDLField(String name, String type, String comment, String value)
    {
        this.name = name;
        this.type = type;
        this.comment = comment;
        this.value = value;
    }
    public IDLField(String name, String type, String comment)
    {
        this(name, type, comment, "");
    }
    public IDLField(String name, String type)
    {
        this(name, type, "", "");
    }
    public IDLField()
    {
        this(null, "", "", "");
    }

    public String getName()
    {
        return name;
    }
    public void setName(String name)
    {
        this.name = name;
    }

    public long getArraySize()
    {
        return arraySize;
    }
    public void setArraySize(long arraySize)
    {
        this.arraySize = arraySize;
    }

    public long getArrayMaxSize()
    {
        return arrayMaxSize;
    }
    public void setArrayMaxSize(long arrayMaxSize)
    {
        this.arrayMaxSize = arrayMaxSize;
    }

    public long getStringSize()
    {
        return stringSize;
    }
    public void setStringSize(long stringSize)
    {
        this.stringSize = stringSize;
    }

    public long getStringMaxSize()
    {
        return stringMaxSize;
    }
    public void setStringMaxSize(long stringMaxSize)
    {
        this.stringMaxSize = stringMaxSize;
    }

    public String getType()
    {
        return type;
    }
    public void setType(String type)
    {
        this.type = type;
    }

    public String getFullyQualifiedType()
    {
        if (fullyQualifiedType == null) return getType();
        if (fullyQualifiedType.equals("")) return getType();
        return fullyQualifiedType;
    }
    public void setFullyQualifiedType(String type)
    {
        this.fullyQualifiedType = type;
    }

    public String getComment()
    {
        return comment;
    }
    public void setComment(String comment)
    {
        this.comment = comment;
    }

    public List<String> getDirectives()
    {
        return this.directives;
    }
    public void setDirectives(List<String> directives)
    {
        this.directives.addAll(directives); 
    }

    public String getValue()
    {
        return value;
    }
    public void setValue(String value)
    {
        this.value = value;
    }

    public String getRangeLo()
    {
        return rangeLo;
    }
    public String getRangeHi()
    {
        return rangeHi;
    }
    public void setRange(String lo, String hi)
    {
        this.rangeLo = lo;
        this.rangeHi = hi;
    }

    public boolean isIdlType()
    {
        return idlType;
    }
    public void setIdlType(boolean value)
    {
        this.idlType = value;
    }

    public boolean isEnumType()
    {
        return enumType;
    }
    public void setEnumType(boolean value)
    {
        this.enumType = value;
    }

    public boolean isBooleanType()
    {
        return booleanType;
    }
    public void setBooleanType(boolean value)
    {
        this.booleanType = value;
    }

    public boolean isIntType()
    {
        return intType;
    }
    public void setIntType(boolean value)
    {
        this.intType = value;
    }

    public boolean isFloatType()
    {
        return floatType;
    }
    public void setFloatType(boolean value)
    {
        this.floatType = value;
    }

    public boolean isStringType()
    {
        return stringType;
    }
    public void setStringType(boolean value)
    {
        this.stringType = value;
    }

    public boolean isArray()
    {
        return type.endsWith("[]");
    }

    public String toString()
    {
        return getType() + " " + getName();
    }

    public boolean isStatic()
    {
        return _static;
    }
    public void setStatic(boolean value)
    {
        this._static = value;
    }

    public boolean isAbstract()
    {
        return _abstract;
    }
    public void setAbstract(boolean value)
    {
        this._abstract = value;
    }

}
