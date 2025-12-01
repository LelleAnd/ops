/*
 * IDLEnumType.java
 *
 */

package parsing;

import java.util.ArrayList;

/**
 *
 * @author lelle
 */
public class IDLEnumType
{
    private String name;
    private String comment;
    private ArrayList<String> enumNames = new ArrayList<String>();

    public IDLEnumType(String name, String comment)
    {
        this.name = name;
        this.comment = comment;
    }

    public IDLEnumType(String name)
    {
        this(name, "");
    }
    public IDLEnumType()
    {
        this(null, "");
    }

    public String getName()
    {
        return name;
    }

    public void setName(String name)
    {
        this.name = name;
    }

    public String getComment()
    {
        return comment;
    }

    public void setComment(String comment)
    {
        this.comment = comment;
    }

    public ArrayList<String> getEnumNames()
    {
        return enumNames;
    }

    public void setEnumNames(ArrayList<String> enumNames)
    {
        this.enumNames = enumNames;
    }

    public String initValue(String suggest)
    {
        if (enumNames.contains(suggest)) return suggest;
        return enumNames.get(0);
    }

}
