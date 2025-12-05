/*
 * JSONCompiler.java
 *
 * Created on den 24 september 2016
 *
 */

package opsc;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Vector;
import parsing.AbstractTemplateBasedIDLCompiler;
import parsing.IDLClass;
import parsing.IDLField;
import parsing.IDLEnumType;

/**
 *
 * @author Lelle
 */
public class JSONCompiler extends CompilerSupport
{
    String res = "";
    public boolean includeVersion = false;

    public JSONCompiler(String projname) {
        super(projname);
        //tab is 2 spaces
        setTabString("  ");
    }

    public void compileDataClasses(Vector<IDLClass> idlClasses, String projectDirectory)
    {
        res = "";
        this._idlClasses = idlClasses;

        res += "[" + endl();
        res += Example_For_describing_alternative_interpretation_for_a_variable(1);
        res += tab(1) + "," + endl();
        res += generate_coretypes_JSON(1);
        res += tab(1) + "," + endl();
        res += generate_OPSObject_JSON(1);
        res += tab(1) + "," + endl();
        res += generate_OPSMessage_JSON(1);
        for (IDLClass iDLClass : idlClasses)
        {
            res += tab(1) + "," + endl();
            res += generateEnumTypeDeclarations(1, iDLClass);
            res += generateJSONobject(1, iDLClass);
        }
        res += "]" + endl();

        //Save the generated text to the output file.
        setOutputFileName(_outputDir + File.separator + _projectName + ".json");
        saveOutputText(res);
        System.out.println("Info: Saved JSON description to file: " + outputFileName);
    }

    protected String Example_For_describing_alternative_interpretation_for_a_variable(int t)
    {
      String res = "";
      res += tab(t) + "{" + endl();
      res += tab(t+1) + "\"an_idea\": {" + endl();
      res += tab(t+2) + "\"desc\": \"An idea of how to describe an alternative interpretation of a specific variable\"," + endl();
      res += tab(t+2) + "\"topic\": \"ExampleDomain::ExampleTopic\"," + endl();
      res += tab(t+2) + "\"variable\": \"sampledata\"," + endl();
      res += tab(t+2) + "\"interpret_as\": [" + endl();
      res += tab(t+3) + "{ \"name\": \"Kalle\", \"type\": \"double\" }," + endl();
      res += tab(t+3) + "{ \"name\": \"Olle\", \"type\": \"int\" }" + endl();
      res += tab(t+2) + "]" + endl();
      res += tab(t+1) + "}" + endl();
      res += tab(t) + "}" + endl();
      return res;
    }

    protected String generate_coretypes_JSON(int t)
    {
        String res = "";
        res += tab(t) + "{" + endl();
        res += tab(t+1) + "\"ops_internals\": {" + endl();
        res += tab(t+2) + "\"coretypes\": [" + endl();
        res += tab(t+3) + "{ \"type\": \"boolean\", \"size\": 1 }," + endl();
        res += tab(t+3) + "{ \"type\": \"byte\", \"size\": 1 }," + endl();
        res += tab(t+3) + "{ \"type\": \"short\", \"size\": 2 }," + endl();
        res += tab(t+3) + "{ \"type\": \"int\", \"size\": 4 }," + endl();
        res += tab(t+3) + "{ \"type\": \"long\", \"size\": 8 }," + endl();
        res += tab(t+3) + "{ \"type\": \"float\", \"size\": 4 }," + endl();
        res += tab(t+3) + "{ \"type\": \"double\", \"size\": 8 }," + endl();

        res += tab(t+3) + "{ \"type\": \"string\"," + endl();
        res += tab(t+4) + "\"composed_of\": [" + endl();
        res += tab(t+5) + "{ \"num_elements\": \"int\" }," + endl();
        res += tab(t+5) + "{ \"elements\": \"byte\" }" + endl();
        res += tab(t+4) + "]" + endl();
        res += tab(t+3) + "}," + endl();

        res += tab(t+3) + "{ \"type\": \"vector<T>\"," + endl();
        res += tab(t+4) + "\"composed_of\": [" + endl();
        res += tab(t+5) + "{ \"num_elements\": \"int\" }," + endl();
        res += tab(t+5) + "{ \"elements\": \"elementtype\" }" + endl();
        res += tab(t+4) + "]" + endl();
        res += tab(t+3) + "}" + endl();

        res += tab(t+2) + "]," + endl();

        res += tab(t+2) + "\"non-coretypes\": {" + endl();
        res += tab(t+3) + "\"composed_of\": [" + endl();
        res += tab(t+4) + "{ \"type_string\": \"string\", \"desc\": \"If the string has a leading '0 ', all types in the inheritance hierarchy are version tagged.\"}," + endl();
        res += tab(t+4) + "{ \"fields\": \"according to type\" }" + endl();
        res += tab(t+3) + "]" + endl();
        res += tab(t+2) + "}," + endl();

        res += tab(t+2) + "\"alignment\": 1," + endl();
        res += tab(t+2) + "\"endianess\": \"little-endian\"" + endl();
        res += tab(t+1) + "}" + endl();
        res += tab(t) + "}" + endl();
        return res;
    }

    private String getVersionField()
    {
        return "{ \"name\": \"version\", \"type\": \"byte\", \"optional\": \"type_is_version_tagged\", \"desc\": \"If not present, 'version' is implicitly 0\" }";
    }

    protected String generate_OPSObject_JSON(int t)
    {
        String res = "";
        res += tab(t) + "{" + endl();
        res += tab(t+1) + "\"type\": \"ops.OPSObject\"," + endl();

        res += tab(t+1) + "\"fields\": [" + endl();
        if (includeVersion) {
            res += tab(t+2) + getVersionField() + "," + endl();
        }
        res += tab(t+2) + "{";
        res += " \"name\": \"key\",";
        res += " \"type\": \"string\"";
        res += " }" + endl();
        res += tab(t+1) + "]" + endl();

        res += tab(t) + "}" + endl();
        return res;
    }

    protected String generate_OPSMessage_JSON(int t)
    {
        String res = "";
        res += tab(t) + "{" + endl();
        res += tab(t+1) + "\"type\": \"ops.protocol.OPSMessage\"," + endl();
        res += tab(t+1) + "\"extends\": \"ops.OPSObject\"," + endl();

        res += tab(t+1) + "\"fields\": [" + endl();
        if (includeVersion) {
            res += tab(t+2) + getVersionField() + "," + endl();
        }
        res += tab(t+2) + "{ \"name\": \"messageType\",       \"type\": \"byte\" }," + endl();
        res += tab(t+2) + "{ \"name\": \"publisherPriority\", \"type\": \"byte\" }," + endl();
        res += tab(t+2) + "{ \"name\": \"publicationID\",     \"type\": \"long\" }," + endl();
        res += tab(t+2) + "{ \"name\": \"publisherName\",     \"type\": \"string\" }," + endl();
        res += tab(t+2) + "{ \"name\": \"topicName\",         \"type\": \"string\" }," + endl();
        res += tab(t+2) + "{ \"name\": \"topLevelKey\",       \"type\": \"string\" }," + endl();
        res += tab(t+2) + "{ \"name\": \"address\",           \"type\": \"string\" }," + endl();
        res += tab(t+2) + "{ \"name\": \"data\",              \"type\": \"virtual ops.OPSObject\" }" + endl();
        res += tab(t+1) + "]" + endl();
        res += tab(t) + "}" + endl();
        return res;
    }

    protected String makeType(IDLField field)
    {
      String ty = field.getType().replace("[]", "");
      if (field.isIdlType()) {
        ty = getFullyQualifiedClassName(ty);
      }
      if (field.isAbstract()) {
        return "virtual " + ty;
      } else {
        return ty;
      }
    }

    protected String validateComment(String comment)
    {
        return comment.replace("/*", "").replace("*/", "").
                replace("\\", "\\\\").
                replace("\b", "\\b").replace("\f", "\\f").
                replace("\n", "\\n").replace("\r", "\\r").
                replace("\t", "\\t").replace("\"", "'").trim();
    }

    protected String generateEnumTypeDeclarations(int t, IDLClass idlClass)
    {
      String ret = "";
      String thisClassName = idlClass.getPackageName() + "." + idlClass.getClassName();
      for (IDLEnumType et : idlClass.getEnumTypes()) {
          ret += tab(t) + "{" + endl();
          ret += tab(t+1) + "\"type\": \"" + thisClassName + "." + et.getName() + "\"," + endl();
          ret += tab(t+1) + "\"basetype\": \"short\"," + endl();

          String comment = validateComment(et.getComment());
          if (comment.length() > 0) {
            ret += tab(t+1) + "\"desc\": \"" + comment + "\"," + endl();
          }
          ret += tab(t+1) + "\"enum_definition\": [" + endl();

          int idx = 0;
          for (String eName : et.getEnumNames()) {
              if (idx > 0) ret += "," + endl();
              ret += tab(t+2) + "{ \"enum\": \"" + eName + "\", \"value\": " + idx + " }";
              idx += 1;
          }
          ret += endl() + tab(t+1) + "]" + endl();
          ret += tab(t) + "}" + endl();
          ret += tab(t) + "," + endl();
      }
      return ret;
    }

    private String getFieldGuard(String versionName, IDLField field)
    {
        String ret = "";
        Vector<VersionEntry> vec = getReducedVersions(field.getName(), field.getDirectives());
        if (vec != null) {
            for (VersionEntry ent : vec) {
                String cond = "{ \"low\": \"" + ent.start + "\", ";
                if (ent.stop != -1) {
                    cond += "\"high\": \"" + ent.stop + "\" }";
                } else {
                    cond += "\"high\": \"255\" }";
                }
                if (ret.length() > 0) {
                    ret += ", ";
                }
                ret += cond;
            }
        }
        return ret;
    }

    protected String generateJSONobject(int t, IDLClass idlClass)
    {
        String res = "";
        String thisClassName = idlClass.getPackageName() + "." + idlClass.getClassName();
        res += tab(t) + "{" + endl();
        res += tab(t+1) + "\"type\": \"" + thisClassName + "\"," + endl();
        String baseClass = "ops.OPSObject";
        if (idlClass.getBaseClassName() != null) {
          baseClass = idlClass.getBaseClassName();
        }
        if (!idlClass.isOnlyDefinition()) {
          res += tab(t+1) + "\"extends\": \"" + getFullyQualifiedClassName(baseClass) + "\"," + endl();
        }

        if (idlClass.getType() == IDLClass.ENUM_TYPE) {
          res += tab(t+1) + "\"enum\": [";
          String pre = "";
          for (String str : idlClass.getEnumNames()) {
            res += pre + "\"" + str + "\"";
            pre = ", ";
          }
          res += "]" + endl();

        } else {

          // Constants
          String consts = tab(t+1) + "\"constants\": [" + endl();
          boolean first = true;
          for (IDLField field : idlClass.getFields()) {
            if (!field.isStatic()) continue;
            if (!first) consts += "," + endl();
            first = false;
            consts += tab(t+2) + "{";
            consts += " \"name\": \"" + field.getName() + "\"";
            consts += ", \"type\": \"" + makeType(field) + "\"";
            consts += ", \"value\": \"" + field.getValue().replace("\"", "'") + "\"";

            String comment = validateComment(field.getComment());
            if (comment.length() > 0) {
              consts += ", \"desc\": \"" + comment + "\"";
            }
            consts += " }";
          }
          consts += endl() + tab(t+1) + "]," + endl();
          if (first == false) res += consts;

          // Fields
          res += tab(t+1) + "\"fields\": [" + endl();
          boolean doneFirst = false;
          if (includeVersion) {
              res += tab(t+2) + getVersionField();
              doneFirst = true;
          }
          for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            if (doneFirst) {
                res += "," + endl();
            }
            doneFirst = true;
            res += tab(t+2) + "{";
            res += " \"name\": \"" + field.getName() + "\"";
            if (field.isArray()) {
              res += ", \"type\": \"vector<T>\", \"elementtype\": ";

            } else {
              res += ", \"type\": ";

            }
            String ttype = makeType(field);
            if (field.isEnumType()) {
              ttype = field.getFullyQualifiedType().replace("[]", "");
            }
            res += "\"" + ttype + "\"";

            String fieldGuard = getFieldGuard("version", field);
            if (fieldGuard.length() > 0) {
                res += ", \"version\": [" + fieldGuard + "]";
            }

            String comment = validateComment(field.getComment());
            if (comment.length() > 0) {
              res += ", \"desc\": \"" + comment + "\"";
            }
            res += " }";
          }
          res += endl() + tab(t+1) + "]" + endl();
        }
        res += tab(t) + "}" + endl();
        return res;
    }

    protected String getLastPart(String name)
    {
      int idx;
      String s = name;
      while ((idx = s.indexOf('.')) > 0) {
        s = s.substring(idx+1);
      }
      return s;
    }

    protected String getFullyQualifiedClassName(String className)
    {
      String s = getLastPart(className);
      for (IDLClass cl : this._idlClasses) {
        if (cl.getClassName().equals(s)) {
          return cl.getPackageName() + "." + cl.getClassName();
        }
      }
      // Didn't find class in list so it is from another "project"
      // Assume then that the 'className' contains <packagename>.<class>
      // and that the unit is <packagename>.<class>, i.e. the same
      return className;
    }

    public String getName()
    {
        return "JSONIDLCompiler";
    }

}
