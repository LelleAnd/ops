/*
 * DelphiCompiler.java
 *
 * Created on den 28 june 2016
 *
 */

package opsc;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Vector;
import java.util.Arrays;
import parsing.AbstractTemplateBasedIDLCompiler;
import parsing.IDLClass;
import parsing.IDLField;
import parsing.IDLEnumType;

/**
 *
 * @author Lelle
 */
public class DelphiCompiler extends opsc.Compiler
{
    final static String UNIT_REGEX = "__unitName";
    final static String IMPORTS_REGEX = "__importUnits";
    final static String CLASS_COMMENT_REGEX = "__classComment";
    final static String CONSTRUCTOR_HEAD_REGEX = "__constructorHead";
    final static String CONSTRUCTOR_BODY_REGEX = "__constructorBody";
    final static String DESTRUCTOR_HEAD_REGEX = "__destructorHead";
    final static String DESTRUCTOR_BODY_REGEX = "__destructorBody";
    final static String DECLARATIONS_REGEX = "__declarations";
    final static String CONSTANTS_REGEX = "__constDeclarations";
    final static String SERIALIZE_HEAD_REGEX = "__serializeHead";
    final static String SERIALIZE_REGEX = "__serialize";
    final static String FILL_CLONE_HEAD_REGEX = "__fillCloneHead";
    final static String FILL_CLONE_BODY_REGEX = "__fillCloneBody";
    final static String VALIDATE_HEAD_REGEX = "__validateHead";
    final static String VALIDATE_BODY_REGEX = "__validateBody";
    final static String SIZE_REGEX = "__size";
    final static String CS_DIR = "Delphi";
//    private String projectDirectory;
    private static String BASE_CLASS_NAME_REGEX = "__baseClassName";
    private static String CREATE_MAKE_BODY_REGEX = "__createMakeBody";

    String createdFiles = "";

    public DelphiCompiler(String projname)
    {
        super(projname);
        //tab is 2 spaces
        setTabString("  ");
    }

    public void compileDataClasses(Vector<IDLClass> idlClasses, String projectDirectory)
    {
        createdFiles = "";
        this._idlClasses = idlClasses;
        //this.projectDirectory = projectDirectory;
        try {
            if (!_onlyGenTypeSupport) {
              for (IDLClass iDLClass : idlClasses) {
                if (iDLClass.getType() == IDLClass.ENUM_TYPE) {
                    compileEnum(iDLClass);
                } else {
                  compileDataClass(iDLClass);
                  // We put the publisher and subscriber in the same file as the data class
                }
              }
            }

            compileTypeSupport(idlClasses, _projectName);
        } catch (IOException iOException)  {
            System.out.println( "Error: Generating Delphi failed with the following exception: " + iOException.getMessage());
        }
    }

    protected void compilePublisher(IDLClass cl) {}
    protected void compileSubscriber(IDLClass cl) {}

    protected void compileEnum(IDLClass idlClass) throws IOException
    {
        String className = getClassName(idlClass);

        String packageName = idlClass.getPackageName();
        String packageFilePart = packageName.replace(".", "/");
        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + packageName + "." + className + ".pas");

        java.io.InputStream stream = findTemplateFile("delphienumtemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
        templateText = templateText.replace(DECLARATIONS_REGEX, getEnumDeclarations(idlClass));

        //Save the modified text to the output file.
        saveOutputText(templateText);
        createdFiles += "\"" + getOutputFileName() + "\"\n";
    }

    private String convertComment(int tnum, String comment)
    {
      String ret = "";
      int idx;
      while ((idx = comment.indexOf('\n')) >= 0) {
          ret += tab(tnum) + "///" + comment.substring(0,idx).replace("/*", "").replace("*/", "").replace("\r", "") + endl();
          comment = comment.substring(idx+1);
      }
      ret += tab(tnum) + "///" + comment.replace("/*", "").replace("*/", "") + endl();
      return ret;
    }

    private String getClassComment(IDLClass idlClass)
    {
        String ret = "";
        if (idlClass.getComment() != null) {
            if (!idlClass.getComment().equals("")) {
                ret = convertComment(1, idlClass.getComment());
            }
        }
        return ret;
    }

    public void compileDataClass(IDLClass idlClass) throws IOException
    {
        String className = getClassName(idlClass);
        String baseClassName = "TOPSObject";
        if (idlClass.getBaseClassName() != null) {
          baseClassName = idlClass.getBaseClassName();

          // Change name for some internal Delphi units
          if (baseClassName.equals("ops.Reply")) baseClassName = "TReply";
          if (baseClassName.equals("ops.Request")) baseClassName = "TRequest";

          baseClassName = getLastPart(baseClassName);
        }
        String packageName = idlClass.getPackageName();

        String packageFilePart = packageName.replace(".", "/");
        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + packageName + "." + className + ".pas");

        java.io.InputStream stream;
        if (isOnlyDefinition(idlClass)) {
          stream = findTemplateFile("delphitemplatebare.tpl");
        } else {
          stream = findTemplateFile("delphitemplate.tpl");
        }
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(IMPORTS_REGEX, getImports(idlClass));
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(CLASS_COMMENT_REGEX, getClassComment(idlClass));
        templateText = templateText.replace(BASE_CLASS_NAME_REGEX, baseClassName);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
        templateText = templateText.replace(CONSTRUCTOR_HEAD_REGEX, getConstructorHead(idlClass));
        templateText = templateText.replace(CONSTRUCTOR_BODY_REGEX, getConstructorBody(idlClass));
        templateText = templateText.replace(DESTRUCTOR_HEAD_REGEX, getDestructorHead(idlClass));
        templateText = templateText.replace(DESTRUCTOR_BODY_REGEX, getDestructorBody(idlClass));
        templateText = templateText.replace(DECLARATIONS_REGEX, getDeclarations(idlClass));
        templateText = templateText.replace(CONSTANTS_REGEX, getConstantDeclarations(idlClass) + getEnumTypeDeclarations(idlClass));
        templateText = templateText.replace(SERIALIZE_HEAD_REGEX, getSerializeHead(idlClass));
        templateText = templateText.replace(SERIALIZE_REGEX, getSerialize(idlClass));
        templateText = templateText.replace(FILL_CLONE_HEAD_REGEX, getFillCloneHead(idlClass));
        templateText = templateText.replace(FILL_CLONE_BODY_REGEX, getFillCloneBody(idlClass));
        templateText = templateText.replace(VALIDATE_HEAD_REGEX, getValidationHead(idlClass));
        templateText = templateText.replace(VALIDATE_BODY_REGEX, getValidationBody(idlClass));

        //Save the modified text to the output file.
        saveOutputText(templateText);
        createdFiles += "\"" + getOutputFileName() + "\"\n";
    }

    public String getName()
    {
        return "DelphiFactoryIDLCompiler";
    }

    protected void compileTypeSupport(Vector<IDLClass> idlClasses, String projectName)
    {
      try {
        String className = projectName + "TypeFactory";
        String packageName = projectName;

        String packageFilePart = packageName.replace(".", "/");
        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + packageName + "." + className + ".pas");

        java.io.InputStream stream = findTemplateFile("delphitypefactorytemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);

        String createBodyText = "";
        String includes = "";

        for (IDLClass iDLClass : idlClasses) {
            if (isOnlyDefinition(iDLClass) || isNoFactory(iDLClass)) continue;

            createBodyText += tab(1) + "if types = '" + iDLClass.getPackageName() + "." + getClassName(iDLClass) + "' then begin" + endl();
            createBodyText += tab(2) + "Result := " + getFullyQualifiedClassName(iDLClass.getClassName()) + ".Create;" + endl();
            createBodyText += tab(2) + "Exit;" + endl();
            createBodyText += tab(1) + "end;" + endl();

            includes += tab(1) + getUnitName(iDLClass.getClassName()) + "," + endl();
        }

        templateText = templateText.replace(CREATE_MAKE_BODY_REGEX, createBodyText);
        templateText = templateText.replace(IMPORTS_REGEX, includes);

        saveOutputText(templateText);

        createdFiles += "\"" + getOutputFileName() + "\"\n";
      } catch (IOException iOException)  {
          System.out.println( "Error: Generating Delphi factory failed with the following exception: " + iOException.getMessage());
      }
    }

    protected String checkReservedName(String name)
    {
        if (isReservedName(name)) return name + "_";
        return name;
    }

    protected String getClassName(IDLClass idlclass)
    {
        return checkReservedName(idlclass.getClassName());
    }

    protected String getFieldName(IDLField field)
    {
        return checkReservedName(field.getName());
    }

    private String elementType(String type)
    {
        return languageType(type.replace("[]", ""));
    }

    protected String convOpsName(String type)
    {
        if (type.equals("OPSObject")) return "TOPSObject";
        if (type.equals("OPSObject[]")) return "TOPSObject[]";
        return type;
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

    protected String getUnitName(String className)
    {
      String s = getLastPart(className);
      for (IDLClass cl : this._idlClasses) {
        if (cl.getClassName().equals(s)) {
          return cl.getPackageName() + "." + getClassName(cl);
        }
      }
      if (className.equals("OPSObject")) { return "uOps.OPSObject"; }
      // Didn't find class in list so it is from another "project"
      // Assume then that the 'className' contains <packagename>.<class>
      // and that the unit is <packagename>.<class>, i.e. the same
      return className;
    }

    protected String getFullyQualifiedClassName(String className)
    {
      String s = elementType(className);
      // We return the unitname.classname
      return getUnitName(s) + "." + convOpsName(checkReservedName(getLastPart(s)));
    }

    protected String getConstructorHead(IDLClass idlClass)
    {
      String ret = "";
      for (IDLField field : idlClass.getFields()) {
          if (field.isStatic()) continue;
          if (field.isIdlType() && field.isArray() && field.getArraySize() > 0) {
              ret += tab(0) + "var" + endl();
              ret += tab(1) +   "i : Integer;";
              break;
          }
      }
      return ret;
    }

    protected String getConstructorBody(IDLClass idlClass)
    {
      String ret = "";
      int version = idlClass.getVersion();
      if (version < 0) { version = 0; }
      if (version > 0) {
          ret += tab(1) + "idlVersionMask := idlVersionMask or 1; // Some value <> 0" + endl();
      }
      ret += tab(1) + getClassName(idlClass) + "_version := " + getClassName(idlClass) + "_idlVersion;" + endl();
      for (IDLField field : idlClass.getFields()) {
          if (field.isStatic()) continue;
          String fieldName = getFieldName(field);
          if (field.isIdlType()) {
              if (field.isArray()) {
                  if (field.getArraySize() > 0) {
                      ret += tab(1) + "for i := 0 to High(" + fieldName + ") do begin" + endl();
                      ret += tab(2) +   fieldName + "[i] := " + getFullyQualifiedClassName(field.getType()) + ".Create;" + endl();
                      ret += tab(1) + "end;" + endl();
                  }
              } else {
                  ret += tab(1) + fieldName + " := " + getFullyQualifiedClassName(field.getType()) + ".Create;" + endl();
              }
          }
      }
      return ret;
    }

    protected String getDestructorHead(IDLClass idlClass)
    {
      String ret = "";
      for (IDLField field : idlClass.getFields()) {
          if (field.isStatic()) continue;
          if (field.isIdlType() && field.isArray()) {
              ret += tab(0) + "var" + endl();
              ret += tab(1) +   "i : Integer;";
              break;
          }
      }
      return ret;
    }

    protected String getDestructorBody(IDLClass idlClass)
    {
      String ret = "";
      for (IDLField field : idlClass.getFields()) {
          if (field.isStatic()) continue;
          String fieldName = getFieldName(field);
          if (field.isIdlType()) {
              if (field.isArray()) {
                ret += tab(1) + "for i := 0 to High(" + fieldName + ") do begin" + endl();
                ret += tab(2) +   "FreeAndNil(" + fieldName + "[i]);" + endl();
                ret += tab(1) + "end;" + endl();
              } else {
                ret += tab(1) + "FreeAndNil(" + fieldName + ");" + endl();
              }
          }
      }
      return ret;
    }

    protected String getFillCloneHead(IDLClass idlClass)
    {
      String ret = "";
      for (IDLField field : idlClass.getFields()) {
          if (field.isStatic()) continue;
          if (field.isIdlType()) {
              if (field.isArray()) {
                ret += tab(0) + "var" + endl();
                ret += tab(1) +   "__i__ : Integer;";
                break;
              }
          }
      }
      return ret;
    }

    private String getFillCloneBody(IDLClass idlClass)
    {
        String ret = "";
        ret += tab(2) + getClassName(idlClass) + "_version := Self." + getClassName(idlClass) + "_version;" + endl();
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            String fieldName = getFieldName(field);
            if (field.isIdlType()) {
                if (!field.isArray()) {
                    // Free existing object and clone the new one
                    ret += tab(2) + "FreeAndNil(" + fieldName + ");" + endl();
                    ret += tab(2) + fieldName + " := " + getFullyQualifiedClassName(field.getType()) + "(Self." + fieldName + ".Clone());" + endl();
                } else {
                    String s = field.getType();
                    s = getFullyQualifiedClassName(s.substring(0, s.indexOf('[')));
                    ret += tab(2) + "for __i__ := 0 to High("+ fieldName + ") do FreeAndNil(" + fieldName + "[__i__]);" + endl();
                    if (field.getArraySize() == 0) {
                        ret += tab(2) + "SetLength(" + fieldName + ", Length(Self." + fieldName + "));" + endl();
                    }
                    ret += tab(2) + "for __i__ := 0 to High("+ fieldName + ") do" + endl();
                    ret += tab(3) +   fieldName + "[__i__] := " + s + "(Self." + fieldName + "[__i__].Clone());" + endl();
                }
            } else if (field.isArray() && (field.getArraySize() == 0)) {
                // Dynamic arrays of base types can be copied using the Delphi global Copy() function
                ret += tab(2) + fieldName + " := Copy(Self." + fieldName + ");" + endl();
            } else {
                // Base types can just be assigned
                ret += tab(2) + fieldName + " := Self." + fieldName + ";" + endl();
            }
        }
        return ret;
    }

    private CharSequence getImports(IDLClass idlClass)
    {
        HashMap<String, String> typesToInclude = new HashMap();
        String ret = "";
        if (idlClass.getBaseClassName() != null) {
            String unit = getUnitName(idlClass.getBaseClassName());

            // Change name for some internal Delphi units
            if (unit.equals("ops.Reply")) unit = "uOps.RequestReply.Reply";
            if (unit.equals("ops.Request")) unit = "uOps.RequestReply.Request";

            typesToInclude.put(unit, unit);
        }
        for (IDLField field : idlClass.getFields()) {
            if (field.isIdlType()) {
                String type = field.getType();
                if (field.isArray()) {
                    type = type.substring(0, type.length() - 2);
                }
                if (type.equals("OPSObject")) continue;
                String unit = getUnitName(type);
                typesToInclude.put(unit, unit);
            }
        }
        for (String s : idlClass.getImports()) {
            String unit = getUnitName(s);
            typesToInclude.put(unit, unit);
        }
        for (String includeType : typesToInclude.values()) {
            ret += tab(1) + includeType + "," + endl();
        }
        return ret;
    }

    private String getEnumDeclarations(IDLClass idlClass)
    {
        String ret = "";
        for (int i = 0; i < idlClass.getEnumNames().size(); i++) {
            ret += idlClass.getEnumNames().get(i);
            if (i < idlClass.getEnumNames().size()-1) {
              ret += ",";
            }
        }
        return ret;
    }

    protected String getDeclareVector(IDLField field)
    {
        String fieldType = getLastPart(field.getType());
        String fieldName = getFieldName(field);
        String ret = fieldName + " : ";
        if (field.getArraySize() == 0) {
            // idl = type[] name;
            if (field.isIdlType()) {
                ret += languageType(getFullyQualifiedClassName(fieldType) + "[]") + ";" + endl();
            } else {
                ret += languageType(fieldType) + ";" + endl();
            }
        } else {
            // idl = type[size] name;
            if (field.isIdlType()) {
                ret += "array [0.." + (field.getArraySize() - 1) + "] of " + languageType(getFullyQualifiedClassName(field.getType())) + ";" + endl();
            } else {
                ret += "array [0.." + (field.getArraySize() - 1) + "] of " + languageType(elementType(field.getType())) + ";" + endl();
            }
        }
        return ret;
    }

    protected String getDeclarations(IDLClass idlClass)
    {
        String ret = "";

        if (!isOnlyDefinition(idlClass)) {
            int version = idlClass.getVersion();
            if (version < 0) { version = 0; }
            // Need an implicit version field that should be [de]serialized
            ret += tab(2) + getClassName(idlClass) + "_version : Byte; " + endl();
        }

        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            String fieldName = getFieldName(field);
            if(!field.getComment().equals("")) {
                ret += convertComment(2, field.getComment());
            }
            String vers = getVersionDescription(field.getDirective());
            if (vers.length() > 0) {
                ret += tab(2) + "/// " + vers + endl();
            }
            String fieldType = field.getType(); ///TEST getLastPart(field.getType());
            if (field.isArray()) {
                ret += tab(2) + getDeclareVector(field);
            } else if(field.getType().equals("string")) {
                ret += tab(2) + fieldName + " : " + languageType(fieldType) + ";" + endl();
            } else if(field.isIdlType()) {
                ret += tab(2) + fieldName + " : " + languageType(getFullyQualifiedClassName(fieldType)) + ";" + endl();
            } else {
                ret += tab(2) + fieldName + " : " + languageType(fieldType) + ";" + endl();
            }
        }
        return ret;
    }

    protected String getConstantDeclarations(IDLClass idlClass)
    {
        String ret = "";
        if (!isOnlyDefinition(idlClass)) {
            int version = idlClass.getVersion();
            if (version < 0) { version = 0; }
            // Need an implicit version field that should be [de]serialized
            ret += tab(3) + getClassName(idlClass) + "_idlVersion : Byte = " + version + ";" + endl();
        }
        for (IDLField field : idlClass.getFields()) {
            if (!field.isStatic()) continue;
            String fieldName = getFieldName(field);
            if (!field.getComment().equals("")) {
                ret += convertComment(3, field.getComment());
            }
            String fieldType = getLastPart(field.getType());
            if (field.getType().equals("string")) {
                ret += tab(3) + fieldName + " = " + languageType(fieldType) + "(" + field.getValue().replace("\"", "'") + ");" + endl();
            } else if (field.getType().equals("float") || (field.getType().equals("double"))) {
                ret += tab(3) + fieldName + " = " + field.getValue() + ";" + endl();
            } else {
                ret += tab(3) + fieldName + " = " + languageType(fieldType) + "(" + field.getValue() + ");" + endl();
            }
        }
        if (!ret.equals("")) ret = tab(2) + "const" + endl() + ret;
        return ret;
    }

    protected String getEnumTypeDeclarations(IDLClass idlClass)
    {
        String ret = "";
        for (IDLEnumType et : idlClass.getEnumTypes()) {
            if (!et.getComment().equals("")) {
                ret += convertComment(3, et.getComment());
            }
            ret += tab(3) + et.getName() + " = (" + endl();
            String values = "";
            for (String eName : et.getEnumNames()) {
                if (values != "") values += ", ";
                values += eName;
            }
            ret += tab(4) + values + endl();
            ret += tab(3) + ");" + endl();
        }
        if (ret != "") {
            ret = tab(2) + "type " + endl() +
                tab(3) + "{$SCOPEDENUMS ON}" + endl() +
                ret +
                tab(3) + "{$SCOPEDENUMS OFF}" + endl();
        }
        return ret;
    }

    protected String getValidationHead(IDLClass idlClass)
    {
      String ret = "";
      for (IDLField field : idlClass.getFields()) {
          if (field.isStatic()) continue;
          if (field.isIdlType() && !field.isAbstract()) {
              if (field.isArray()) {
                ret += tab(0) + "var" + endl();
                ret += tab(1) +   "__i__ : Integer;";
                break;
              }
          }
      }
      return ret;
    }

    protected String getValidationBody(IDLClass idlClass)
    {
      String ret = "";
      for (IDLField field : idlClass.getFields()) {
          if (field.isStatic()) continue;
          String fieldName = getFieldName(field);
          String fieldType = getLastPart(field.getType());
          if (field.isIdlType() && !field.isAbstract()) {
              // 'virtual': All fields that are objects are also virtual in Delphi!!
              // Need to validate that an object that isn't declared 'virtual' really
              // is of the correct type
              if (field.isArray()) {
                String s = field.getType();
                s = convOpsName(getLastPart(s.substring(0, s.indexOf('['))));
                ret += tab(1) + "for __i__ := 0 to High(" + fieldName + ") do begin" + endl();
                ret += tab(2) + "if not " + fieldName + "[__i__].ClassNameIs('" + s + "') then Result := False;" + endl();
                ret += tab(1) + "end;" + endl();
              } else {
                ret += tab(1) + "if not " + fieldName + ".ClassNameIs('" + getLastPart(field.getType()) + "') then Result := False;" + endl();
              }
          }
      }
      return ret;
    }

    protected String languageType(String s)
    {
      if (s.equals("string"))    return "AnsiString";
      if (s.equals("boolean"))   return "Boolean";
      if (s.equals("int"))       return "Int32";
      if (s.equals("short"))     return "Int16";
      if (s.equals("long"))      return "Int64";
      if (s.equals("double"))    return "Double";
      if (s.equals("float"))     return "Single";
      if (s.equals("byte"))      return "Byte";
      if (s.equals("string[]"))  return "TDynAnsiStringArray";
      if (s.equals("int[]"))     return "TDynInt32Array";
      if (s.equals("short[]"))   return "TDynInt16Array";
      if (s.equals("long[]"))    return "TDynInt64Array";
      if (s.equals("double[]"))  return "TDynDoubleArray";
      if (s.equals("float[]"))   return "TDynSingleArray";
      if (s.equals("byte[]"))    return "TDynByteArray";
      if (s.equals("boolean[]")) return "TDynBooleanArray";
      if (s.endsWith("[]"))      return "array of " + s.substring(0, s.indexOf('['));
      return s;
    }

    protected String getSerializeHead(IDLClass idlClass)
    {
        String ret = "";
        boolean enumFound = false;
        boolean arrayFound = false;
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            if (field.isEnumType()) {
                enumFound = true;
                if (field.isArray()) arrayFound = true;
            }
        }
        if (enumFound) {
            ret += "var" + endl();
            if (arrayFound) {
                ret += tab(1) + "__i__, __num__ : Integer;" + endl();
            }
            ret += tab(1) + "__tmp__ : Int16;";
        }
        return ret;
    }

    private String getFieldGuard(String versionName, IDLField field)
    {
        String ret = "";
        Vector<VersionEntry> vec = getReducedVersions(field.getName(), field.getDirective());
        if (vec != null) {
            for (VersionEntry ent : vec) {
                String cond = "(" + versionName + " >= " + ent.start + ")";
                if (ent.stop != -1) {
                    cond = "(" + cond + " and (" + versionName + " <= " + ent.stop + "))";
                }
                if (ret.length() > 0) {
                    ret += " or ";
                }
                ret += cond;
            }
        }
        return ret;
    }

    protected String getSerialize(IDLClass idlClass)
    {
        String ret = "";
        String versionName = getClassName(idlClass) + "_version";
        String versionNameIdl = getClassName(idlClass) + "_idlVersion";
        // Need an implicit version field that may be [de]serialized
        ret += tab(1) + "if idlVersionMask <> 0 then begin" + endl();
        ret += tab(2) + "archiver.Inout('" + versionName + "', " + versionName + ");" + endl();
        ret += tab(2) + "if " + versionName + " > " + versionNameIdl + " then begin" + endl();
        ret += tab(3) + "raise EIdlVersionException.Create('" + getClassName(idlClass) + "', " + versionName + ", " + versionNameIdl + ");" + endl();
        ret += tab(2) + "end;" + endl();
        ret += tab(1) + "end else begin" + endl();
        ret += tab(2) + versionName + " := 0;" + endl();
        ret += tab(1) + "end;" + endl();
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            String fieldName = getFieldName(field);
            String fieldGuard = getFieldGuard(versionName, field);
            int tabCnt = 1;
            if (fieldGuard.length() > 0) {
                ret += tab(tabCnt) + "if " + fieldGuard + " then begin" + endl();
                tabCnt += 1;
            }
            ret += tab(tabCnt);
            if (field.isIdlType()) {
                if (!field.isArray()) {
                  if (field.isAbstract()) {
                    ret += fieldName + " := " + getFullyQualifiedClassName(field.getType()) +
                          "(archiver.Inout2('" + field.getName() + "', TSerializable(" + fieldName + ")));" + endl();
                  } else {
                    ret += "archiver.Inout('" + field.getName() + "', TSerializable(" + fieldName + "));" + endl();
                  }
                } else {
                    String isAbsType = "false";
                    if (field.isAbstract()) {
                      isAbsType = "true";
                    }
                    if (field.getArraySize() > 0) {
                        // idl = type[size] name;
                        // TestDataArchiveHelper.inoutfixarr(archiver, 'ftest2s', ftest2s, 5);
                        ret += getFullyQualifiedClassName(field.getType()) +
                              "ArchiveHelper.Inoutfixarr(archiver, '" + fieldName + "', " + fieldName + ", " +
                              field.getArraySize() + ", " + isAbsType + ");" + endl();
                    } else {
                        ret += getFullyQualifiedClassName(field.getType()) +
                              "ArchiveHelper.Inoutdynarr(archiver, '" + fieldName + "', TDynSerializableArray(" + fieldName +
                              "), " + isAbsType + ");" + endl();
///                        ret += "archiver.Inout('" + field.getName() + "', TDynSerializableArray(" + fieldName + "));" + endl();
                    }
                }
            } else if (field.isEnumType()) {
                // Enum type
                if (field.isArray()) {
                    ret += "__num__ := archiver.beginList('" + field.getName() + "', ";
                    if (field.getArraySize() == 0) {
                        ret += "Length(" + fieldName + "));" + endl();
                        ret += tab(1) + "if Length(" + fieldName + ") <> __num__ then Setlength(" + fieldName + ", __num__);" + endl();
                    } else {
                        ret += field.getArraySize() + ");" + endl();
                        ret += tab(1) + "if __num__ <> " + field.getArraySize() + " then raise EArchiverException.Create('Illegal size of fix array received. name: " + fieldName + "');" + endl();
                    }
                    ret += tab(1) + "for __i__ := 0 to __num__-1 do begin" + endl();
                    fieldName += "[__i__]";
                    ret += tab(2);
                }
                ret += "__tmp__ := Int16(" + fieldName + "); archiver.Inout('" + field.getName() + "', __tmp__); " + fieldName + " := " + elementType(field.getType()) + "(__tmp__); " + endl();
                if (field.isArray()) {
                    ret += tab(1) + "end;" + endl();
                    ret += tab(1) + "archiver.endList('" + field.getName() + "');" + endl();
                }
            } else {
                // core types
                if (field.isArray()) {
                    if (field.getArraySize() > 0) {
                        // idl = type[size] name;
                        if (field.getType().equals("string[]")) {
                            ret += "archiver.Inoutfixarr('" + field.getName() + "', " + fieldName + ", " + field.getArraySize() + ");" + endl();
                        } else {
                            ret += "archiver.Inoutfixarr('" + field.getName() + "', @" + fieldName + "[0], " +
                                field.getArraySize() + ", SizeOf(" + fieldName + "));" + endl();
                        }
                    } else {
                        // idl = type[] name;
                        ret += "archiver.Inout('" + field.getName() + "', " + fieldName + ");" + endl();
                    }
                } else {
                  ret += "archiver.Inout('" + field.getName() + "', " + fieldName + ");" + endl();
                }
            }
            if (fieldGuard.length() > 0) {
                tabCnt -= 1;
                ret += tab(tabCnt) + "end;" + endl();
            }
        }
        return ret;
    }

    public boolean isReservedName(String name)
    {
        return Arrays.binarySearch(reservedNames, name.toLowerCase()) >= 0;
    }

    // Array of all reserved keywords in ascending order (for binarySearch() to work)
    private static final String[] reservedNames = {
      "and", "array", "as", "asm",
      "begin",
      "case", "class", "const", "constructor",
      "destructor", "dispinterface", "div", "do", "downto",
      "else", "end", "except", "exports",
      "file", "finalization", "finally", "for", "function",
      "goto",
      "if", "implementation", "in", "inherited", "initialization", "inline",
      "interface", "is",
      "label", "library",
      "mod",
      "nil", "not",
      "object", "of", "or", "out",
      "packed", "procedure", "program", "property",
      "raise", "record", "repeat", "resourcestring", "result",
      "set", "shl", "shr", "string",
      "then", "threadvar", "to", "try", "type",
      "unit", "until", "uses",
      "var",
      "while", "with",
      "xor"
    };

}
