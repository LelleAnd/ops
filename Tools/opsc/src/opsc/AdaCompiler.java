/*
 * AdaCompiler.java
 *
 * Created on den 28 november 2016
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
public class AdaCompiler extends opsc.Compiler
{
    final static String UNIT_REGEX = "__unitName";
    final static String UNIT_PUB_REGEX = "__pubUnitName";
    final static String UNIT_SUB_REGEX = "__subUnitName";
    final static String IMPORTS_REGEX = "__importUnits";
    final static String CLASS_COMMENT_REGEX = "__classComment";
    final static String CONSTRUCTOR_HEAD_REGEX = "__constructorHead";
    final static String CONSTRUCTOR_BODY_REGEX = "__constructorBody";
    final static String DESTRUCTOR_HEAD_REGEX = "__destructorHead";
    final static String DESTRUCTOR_BODY_REGEX = "__destructorBody";
    final static String CONSTANTS_REGEX = "__constDeclarations";
    final static String DECLARATIONS_REGEX = "__declarations";
    final static String SERIALIZE_REGEX = "__serialize";
    final static String FILL_CLONE_HEAD_REGEX = "__fillCloneHead";
    final static String FILL_CLONE_BODY_REGEX = "__fillCloneBody";
    final static String VALIDATE_HEAD_REGEX = "__validateHead";
    final static String VALIDATE_BODY_REGEX = "__validateBody";
    final static String SIZE_REGEX = "__size";
    final static String CS_DIR = "Ada";
//    private String projectDirectory;
    private static String BASE_CLASS_NAME_REGEX = "__baseClassName";
    private static String CREATE_MAKE_BODY_REGEX = "__createMakeBody";
    private static String PROJNAME_REGEX = "__projName";
    private static String PROJDEPEND_REGEX = "__projDepend";

    private boolean alwaysDynArray = true;
    private boolean alwaysDynObject = true;

    String ops4GprPath = "";
    String createdFiles = "";

    public AdaCompiler(String projname)
    {
        super(projname);
        //tab is 2 spaces
        setTabString("  ");
    }

    public String getName()
    {
        return "AdaFactoryIDLCompiler";
    }

    public void compileDataClasses(Vector<IDLClass> idlClasses, String projectDirectory)
    {
        createdFiles = "";
        this._idlClasses = idlClasses;
//        this.projectDirectory = projectDirectory;
        try {
            if (!_onlyGenTypeSupport) {
              for (IDLClass iDLClass : idlClasses) {
                if (iDLClass.getType() == IDLClass.ENUM_TYPE) {
                  compileEnum(iDLClass);
                } else {
                  compileDataClass(iDLClass);
                  if (!isOnlyDefinition(iDLClass)) {
                    if (!isTopLevel(iDLClass)) {
                      System.out.println("Info: Ada, skipping generation of publisher/subscriber for " + iDLClass.getClassName());
                    } else {
                      compileSubscriber(iDLClass);
                      compilePublisher(iDLClass);
                    }
                  }
                }
              }
            }
            compileTypeSupport(idlClasses, _projectName);
            if (!_onlyGenTypeSupport) compileProjectFile(_projectName);
        } catch (IOException iOException)  {
            System.out.println( "Error: Generating Ada failed with the following exception: " + iOException.getMessage());
        }
    }

    protected void compilePubSubHelper(String className, String templateName, String fileName) throws IOException
    {
      setOutputFileName(fileName);

      java.io.InputStream stream = findTemplateFile(templateName);
      setTemplateTextFromResource(stream);

      //Get the template file as a String
      String templateText = getTemplateText();

      //Replace regular expressions in the template file.
      templateText = templateText.replace(UNIT_REGEX, getUnitName(className));
      templateText = templateText.replace(UNIT_PUB_REGEX, getPubUnitName(className));
      templateText = templateText.replace(UNIT_SUB_REGEX, getSubUnitName(className));
      templateText = templateText.replace(CLASS_NAME_REGEX, className);

      //Save the modified text to the output file.
      saveOutputText(templateText);
      createdFiles += "\"" + getOutputFileName() + "\"\n";
    }

    protected void compilePublisher(IDLClass idlClass) throws IOException
    {
      String className = idlClass.getClassName();
      String packageName = idlClass.getPackageName();
      String packageFilePart = packageName.replace(".", "/");
      String baseFileName = _outputDir + File.separator + packageFilePart + File.separator + getPubUnitName(className).replace(".", "-");
      compilePubSubHelper(className, "adaspecpublishertemplate.tpl", baseFileName + ".ads");
      compilePubSubHelper(className, "adabodypublishertemplate.tpl", baseFileName + ".adb");
    }

    protected void compileSubscriber(IDLClass idlClass) throws IOException
    {
      String className = idlClass.getClassName();
      String packageName = idlClass.getPackageName();
      String packageFilePart = packageName.replace(".", "/");
      String baseFileName = _outputDir + File.separator + packageFilePart + File.separator + getSubUnitName(className).replace(".", "-");
      compilePubSubHelper(className, "adaspecsubscribertemplate.tpl", baseFileName + ".ads");
      compilePubSubHelper(className, "adabodysubscribertemplate.tpl", baseFileName + ".adb");
    }

    protected void compileEnumHelper(IDLClass idlClass, String className, String packageName, String templateName, String fileName) throws IOException
    {
      setOutputFileName(fileName);

      java.io.InputStream stream = findTemplateFile(templateName);
      setTemplateTextFromResource(stream);

      //Get the template file as a String
      String templateText = getTemplateText();

      //Replace regular expressions in the template file.
      templateText = templateText.replace(UNIT_REGEX, getUnitName(className));
      templateText = templateText.replace(CLASS_NAME_REGEX, className);
      templateText = templateText.replace(CLASS_COMMENT_REGEX, getClassComment(idlClass));
      templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
      templateText = templateText.replace(DECLARATIONS_REGEX, getEnumDeclarations(idlClass));

      //Save the modified text to the output file.
      saveOutputText(templateText);
      createdFiles += "\"" + getOutputFileName() + "\"\n";
    }

    protected void compileEnum(IDLClass idlClass) throws IOException
    {
        String className = idlClass.getClassName();
        String packageName = idlClass.getPackageName();
        String packageFilePart = packageName.replace(".", "/");
        String baseFileName = _outputDir + File.separator + packageFilePart + File.separator + getUnitName(className).replace(".", "-");

        compileEnumHelper(idlClass, className, packageName, "adaspecenumtemplate.tpl", baseFileName + ".ads");
        compileEnumHelper(idlClass, className, packageName, "adabodyenumtemplate.tpl", baseFileName + ".adb");
    }

    private String getClassComment(IDLClass idlClass)
    {
        String ret = "";
        if (idlClass.getComment() != null) {
            if (!idlClass.getComment().equals("")) {
                String comment = idlClass.getComment();
                int idx;
                while ((idx = comment.indexOf('\n')) >= 0) {
                    ret += tab(1) + "--- " + comment.substring(0,idx).replace("/*", "").replace("*/", "").replace("\r", "") + endl();
                    comment = comment.substring(idx+1);
                }
                ret += tab(1) + "--- " + comment.replace("/*", "").replace("*/", "") + endl();
            }
        }
        return ret;
    }

    public void compileDataClass(IDLClass idlClass) throws IOException
    {
        String className = idlClass.getClassName();
        String baseClassName = "OpsObject";
        if (idlClass.getBaseClassName() != null) {
          baseClassName = idlClass.getBaseClassName();

          // Change name for some internal Delphi units
          if (baseClassName.equals("ops.Reply")) baseClassName = "TReply";
          if (baseClassName.equals("ops.Request")) baseClassName = "TRequest";

          baseClassName = getLastPart(baseClassName);
        }
        String packageName = idlClass.getPackageName();
        String packageFilePart = packageName.replace(".", "/");
        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + getUnitName(className).replace(".", "-") + ".ads");

        java.io.InputStream stream;
        if (isOnlyDefinition(idlClass)) {
          stream = findTemplateFile("adaspectemplatebare.tpl");
        } else {
          stream = findTemplateFile("adaspectemplate.tpl");
        }
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(UNIT_REGEX, getUnitName(className));
        templateText = templateText.replace(IMPORTS_REGEX, getImports(idlClass));
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(CLASS_COMMENT_REGEX, getClassComment(idlClass));
        templateText = templateText.replace(BASE_CLASS_NAME_REGEX, baseClassName);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
        templateText = templateText.replace(DECLARATIONS_REGEX, getDeclarations(idlClass));
        templateText = templateText.replace(CONSTANTS_REGEX, getConstantDeclarations(idlClass) + endl() + getEnumTypeDeclarations(idlClass));

        //Save the modified text to the output file.
        saveOutputText(templateText);
        createdFiles += "\"" + getOutputFileName() + "\"\n";

        if (isOnlyDefinition(idlClass)) return;

        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + getUnitName(className).replace(".", "-") + ".adb");

        stream = findTemplateFile("adabodytemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(UNIT_REGEX, getUnitName(idlClass.getClassName()));
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(BASE_CLASS_NAME_REGEX, baseClassName);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
        templateText = templateText.replace(CONSTRUCTOR_BODY_REGEX, getConstructorBody(idlClass));
        templateText = templateText.replace(DESTRUCTOR_BODY_REGEX, getDestructorBody(idlClass));
        templateText = templateText.replace(SERIALIZE_REGEX, getSerialize(idlClass));
        templateText = templateText.replace(FILL_CLONE_BODY_REGEX, getFillCloneBody(idlClass));

        //Save the modified text to the output file.
        saveOutputText(templateText);
        createdFiles += "\"" + getOutputFileName() + "\"\n";
    }

    protected void compileTypeSupport(Vector<IDLClass> idlClasses, String projectName)
    {
      try {
        String className = projectName + "TypeFactory";
        String packageName = projectName;
        String unitNamePart = "Ops_Pa.SerializableFactory_Pa." + packageName + "_" + className;

        String packageFilePart = packageName.replace(".", "/");

        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + unitNamePart.replace(".", "-") + ".ads");

        java.io.InputStream stream = findTemplateFile("adaspectypefactorytemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(UNIT_REGEX, unitNamePart);
        templateText = templateText.replace(CLASS_NAME_REGEX, className);

        saveOutputText(templateText);
        createdFiles += "\"" + getOutputFileName() + "\"\n";

        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + unitNamePart.replace(".", "-") + ".adb");

        stream = findTemplateFile("adabodytypefactorytemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(UNIT_REGEX, unitNamePart);
        templateText = templateText.replace(CLASS_NAME_REGEX, className);

        String createBodyText = "";
        String includes = "";

        for (IDLClass iDLClass : idlClasses) {
            if (isOnlyDefinition(iDLClass) || isNoFactory(iDLClass)) continue;

            createBodyText += tab(2) + "if types = \"" + iDLClass.getPackageName() + "." + iDLClass.getClassName() + "\" then" + endl();
            createBodyText += tab(3) +   "return Serializable_Class_At(" + getUnitName(iDLClass.getClassName()) + ".Create);" + endl();
            createBodyText += tab(2) + "end if;" + endl();

            includes += tab(1) + getUnitName(iDLClass.getClassName()) + "," + endl();
        }

        templateText = templateText.replace(CREATE_MAKE_BODY_REGEX, createBodyText);
        templateText = templateText.replace(IMPORTS_REGEX, includes);

        saveOutputText(templateText);
        createdFiles += "\"" + getOutputFileName() + "\"\n";
      } catch (IOException iOException)  {
          System.out.println( "Error: Generating Ada Factory failed with the following exception: " + iOException.getMessage());
      }
    }

    protected void compileProjectFile(String projectName) throws IOException
    {
      setOutputFileName(_outputDir + File.separator + projectName + ".gpr");

      java.io.InputStream stream = findTemplateFile("adaprojectfiletemplate.tpl");
      setTemplateTextFromResource(stream);

      //Get the template file as a String
      String templateText = getTemplateText();

      // Default dependency
      String projectDependencies = "-- This requires that environment symbol GPR_PROJECT_PATH includes path to ops4.gpr file" + endl();
      if (ops4GprPath != "") {
        projectDependencies = "-- Path to ops4.gpr file from command line when generating files" + endl();
      }
      projectDependencies += "with \"" + ops4GprPath + "ops4.gpr\";";

      //Replace regular expressions in the template file.
      templateText = templateText.replace(PROJNAME_REGEX, projectName);
      templateText = templateText.replace(PROJDEPEND_REGEX, projectDependencies);

      saveOutputText(templateText);
      createdFiles += "\"" + getOutputFileName() + "\"\n";
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
      // Inheritance chain:
      //   OpsObject
      //   sds.MessageHeaderData
      //   pizza.PizzaData
      //   pizza.VessuvioData
      //   pizza.CapricosaData
      //   pizza.special.LHCData
      //   pizza.special.ExtraAllt

      // Put all units below Ops_Pa.OpsObject_Pa
      // Ops_Pa.OpsObject_Pa.sds_MessageHeaderData
      // Ops_Pa.OpsObject_Pa.pizza_PizzaData
      // Ops_Pa.OpsObject_Pa.pizza_special_ExtraAllt
      String baseUnit = "Ops_Pa.OpsObject_Pa.";

      String s = getLastPart(className);
      for (IDLClass cl : this._idlClasses) {
        if (cl.getClassName().equals(s)) {
          String unit = cl.getPackageName() + "." + cl.getClassName();
          return baseUnit + unit.replace(".", "_");
        }
      }
      // Didn't find class in list so it is from another "project"
      // Assume then that the 'className' contains <packagename>.<class>
      // and that the unit is <packagename>.<class>, i.e. the same
      return baseUnit + className.replace(".", "_");
    }

    protected String getPubUnitName(String className)
    {
      return getUnitName(className).replace("Ops_Pa.OpsObject_Pa.", "Ops_Pa.PublisherAbs_Pa.Publisher_Pa.");
    }

    protected String getSubUnitName(String className)
    {
      return getUnitName(className).replace("Ops_Pa.OpsObject_Pa.", "Ops_Pa.Subscriber_Pa.");
    }

    protected String getFullyQualifiedClassName(String className)
    {
      // We return the unitname.classname
      return getUnitName(className) + "." + getLastPart(className) + "_Class";
    }

//    protected String getConstructorHead(IDLClass idlClass)
//    {
//      String ret = "";
//      return ret;
//    }

    protected String getConstructorBody(IDLClass idlClass)
    {
      String ret = "";
      if (idlClass.getVersion() > 0) {
          ret += tab(2) + "Self.SetIdlVersionMask(Self.IdlVersionMask or UInt32(1)); -- Some value /= 0" + endl();
      }
      for (IDLField field : idlClass.getFields()) {
          if (field.isIdlType() && !field.isArray()) {
              if (alwaysDynObject || field.isAbstract()) {
                  ret += tab(2) + "Self." + getFieldName(field) + " := Create;" + endl();
              }
          }
      }
      return ret;
    }

//    protected String getDestructorHead(IDLClass idlClass)
//    {
//      String ret = "";
//      return ret;
//    }

    protected String genFree(int pos, String name)
    {
      String ret = "";
      ret += tab(pos) + "if " + name + " /= null then" + endl();
      ret += tab(pos+1) + "Free(" + name + ");" + endl();
      ret += tab(pos) + "end if;" + endl();
      return ret;
    }

    protected String getDestructorBody(IDLClass idlClass)
    {
      String ret = "";
      int pos = 2;
      for (IDLField field : idlClass.getFields()) {
          if (field.isStatic()) continue;
          if (field.isIdlType()) {
              if (field.isArray()) {
                  if (alwaysDynArray || (field.getArraySize() == 0)) {
                      ret += tab(pos++) + "if Self." + getFieldName(field) + " /= null then" + endl();
                  }
                  if (alwaysDynObject || field.isAbstract()) {
                      ret += tab(pos) + "Clear(Self." + getFieldName(field) + ".all);" + endl();

                      //ret += tab(pos) + "for i in Self." + getFieldName(field) + "'Range loop" + endl();
                      //ret += genFree(pos+1, "Self." + getFieldName(field) + "(i)");
                      //ret += tab(pos) + "end loop;" + endl();
                  }
                  if (alwaysDynArray || (field.getArraySize() == 0)) {
                      ret += tab(pos) + "Dispose(Self." + getFieldName(field) + ");" + endl();
                      ret += tab(--pos) + "end if;" + endl();
                  }
              } else {
                  if (alwaysDynObject || field.isAbstract()) {
                      ret += genFree(pos, "Self." + getFieldName(field));
                  }
              }
          } else {
              // Core types
              if (field.isArray()) {
                  if (alwaysDynArray || (field.getArraySize() == 0)) {
                      ret += tab(pos++) + "if Self." + getFieldName(field) + " /= null then" + endl();
                  }
                  if (elementType(field.getType()) == "String_At") {
                      ret += tab(pos) + "Clear(Self." + getFieldName(field) + ".all);" + endl();

                      //ret += tab(pos) + "for i in Self." + getFieldName(field) + "'Range loop" + endl();
                      //ret += tab(pos+1) + "Dispose(Self." + getFieldName(field) + "(i));" + endl();
                      //ret += tab(pos) + "end loop;" + endl();
                  }
                  if (alwaysDynArray || (field.getArraySize() == 0)) {
                      ret += tab(pos) + "Dispose(Self." + getFieldName(field) + ");" + endl();
                      ret += tab(--pos) + "end if;" + endl();
                  }
              } else {
                  if (languageType(field.getType()) == "String_At") {
                      ret += tab(pos) + "if Self." + getFieldName(field) + " /= null then" + endl();
                      ret += tab(pos+1) +   "Dispose(Self." + getFieldName(field) + ");" + endl();
                      ret += tab(pos) + "end if;" + endl();
                  }
              }
          }
      }
      return ret;
    }

//    protected String getFillCloneHead(IDLClass idlClass)
//    {
//      String ret = "";
//      return ret;
//    }

    private String getFillCloneBody(IDLClass idlClass)
    {
        String ret = "";
        int pos = 3;
        String className = idlClass.getClassName() + "_Class";
        ret += tab(pos) + className + "(obj.all)." + idlClass.getClassName() + "_version := Self." + idlClass.getClassName() + "_version;" + endl();
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            String fieldName = getFieldName(field);
            if (field.isIdlType()) {
                // Idl types
                if (field.isArray()) {
                    String elemType = getLastPart(elementType(field.getType()));
                    // Remove ev old elements and array if dynamic
                    if (alwaysDynArray || (field.getArraySize() == 0)) {
                        ret += tab(pos) + "if " + className + "(obj.all)." + fieldName + " /= null then" + endl();
                        pos++;
                    }
                    if (alwaysDynObject || field.isAbstract()) {
                        ret += tab(pos) + "Clear(" + className + "(obj.all)." + fieldName + ".all);" + endl();

                        //ret += tab(pos) + "for i in " + className + "(obj.all)." + fieldName + "'Range loop" + endl();
                        //ret += genFree(pos+1, className + "(obj.all)." + fieldName + "(i)");
                        //ret += tab(pos) + "end loop;" + endl();
                    }
                    if (alwaysDynArray || (field.getArraySize() == 0)) {
                        ret += tab(pos) + "Dispose(" + className + "(obj.all)." + fieldName + ");" + endl();
                        pos--;
                        ret += tab(pos) + "end if;" + endl();
                    // Create new array if source not null
                        ret += tab(pos) + "if Self." + fieldName + " /= null then" + endl();
                        pos++;
                        if (alwaysDynObject || field.isAbstract()) {
                            ret += tab(pos) + className + "(obj.all)." + fieldName + " := new " + elemType + "_Class_At_Arr(Self." + fieldName + "'Range);" + endl();
                        } else {
                            ret += tab(pos) + className + "(obj.all)." + fieldName + " := new " + elemType + "_Class_Arr(Self." + fieldName + "'Range);" + endl();
                        }
                    }
                    // Clone source
                    ret += tab(pos) + "for i in Self." + fieldName + "'Range loop" + endl();
                    if (alwaysDynObject || field.isAbstract()) {
                        ret += tab(pos+1) + "if Self." + fieldName + "(i) /= null then" + endl();
                        ret += tab(pos+2) + className + "(obj.all)." + fieldName + "(i) := " + elemType + "_Class_At(Clone(Self." + fieldName + "(i).all));" + endl();
                        ret += tab(pos+1) + "end if;" + endl();
                    } else {
                        //FillClone(Self.test2s2(i), ChildData_Class(obj.all).test2s2(i)'Access);
                        ret += tab(pos+1) + "FillClone(Self." + fieldName + "(i), " + className + "(obj.all)." + fieldName + "(i)'Access);" + endl();
                    }
                    ret += tab(pos) + "end loop;" + endl();
                    if (alwaysDynArray || (field.getArraySize() == 0)) {
                        pos--;
                        ret += tab(pos) + "end if;" + endl();
                    }
                } else {
                    if (alwaysDynObject || field.isAbstract()) {
                        // Free existing object and clone the new one
                        ret += genFree(pos, className + "(obj.all)." + fieldName);
                        ret += tab(pos) + "if Self." + fieldName + " /= null then" + endl();
                        ret += tab(pos+1) + className + "(obj.all)." + fieldName + " := " + getFullyQualifiedClassName(field.getType()) + "_At(Clone(Self." + fieldName + ".all));" + endl();
                        ret += tab(pos) + "end if;" + endl();
                    } else {
                        //FillClone(Self.test2, ChildData_Class(obj.all).test2'Access);
                        ret += tab(pos) + "FillClone(Self." + fieldName + ", " + className + "(obj.all)." + fieldName + "'Access);" + endl();
                    }
                }
            } else {
                // Core types
                if (field.isArray()) {
                    if ((!alwaysDynArray) && (field.getArraySize() > 0)) {
                        // Fixed arrays of core types, just assign except for strings that need Replace
                        if (elementType(field.getType()) == "String_At") {
                            ret += tab(pos) + "for i in Self."+ fieldName + "'Range loop" + endl();
                            ret += tab(pos+1) +   "Replace(" + className + "(obj.all)." + fieldName + "(i), Self." + fieldName + "(i));" + endl();
                            ret += tab(pos) + "end loop;" + endl();
                        } else {
                            ret += tab(pos) + className + "(obj.all)." + fieldName + " := Self." + fieldName + ";" + endl();
                        }
                    } else {
                        // dynamic arrays of core types
                        if (elementType(field.getType()) == "String_At") {
                            ret += tab(pos) + "Clear(" + className + "(obj.all)." + fieldName + ");" + endl();
                            ret += tab(pos) + "Dispose(" + className + "(obj.all)." + fieldName + ");" + endl();
                            ret += tab(pos) + "if Self." + fieldName + " /= null then" + endl();
                            ret += tab(pos+1) + className + "(obj.all)." + fieldName + " := new String_Arr(Self." + fieldName + "'Range);" + endl();
                            ret += tab(pos+1) + "for i in Self." + fieldName + "'Range loop" + endl();
                            ret += tab(pos+2) +   className + "(obj.all)." + fieldName + "(i) := Copy(Self." + fieldName + "(i));" + endl();
                            ret += tab(pos+1) + "end loop;" + endl();
                            ret += tab(pos) + "end if;" + endl();
                        } else {
                            ret += tab(pos) + "Dispose(" + className + "(obj.all)." + fieldName + ");" + endl();
                            ret += tab(pos) + "if Self." + fieldName + " /= null then" + endl();
                            ret += tab(pos+1) + className + "(obj.all)." + fieldName + " := new " + elementType(getLastPart(field.getType())) + "_Arr'(Self." + fieldName + ".all);" + endl();
                            ret += tab(pos) + "end if;" + endl();
                        }
                    }
                } else {
                    // Non array core types, just assign except for strings that need Replace()
                    if (elementType(field.getType()) == "String_At") {
                        ret += tab(pos) + "Replace(" + className + "(obj.all)." + fieldName + ", " + "Self." + fieldName + ");" + endl();
                    } else {
                        ret += tab(pos) + className + "(obj.all)." + fieldName + " := Self." + fieldName + ";" + endl();
                    }
                }
            }
        }
        if (ret == "") ret = tab(pos) + "null;";
        return ret;
    }

    private CharSequence getImports(IDLClass idlClass)
    {
        HashMap<String, String> typesToInclude = new HashMap();
        String ret = "";
        if (idlClass.getBaseClassName() != null) {
            String unit = getUnitName(idlClass.getBaseClassName());

///TODO            // Change name for some internal Delphi units
///            if (unit.equals("ops.Reply")) unit = "uOps.RequestReply.Reply";
///            if (unit.equals("ops.Request")) unit = "uOps.RequestReply.Request";

            typesToInclude.put(unit, unit);
        }
        for (IDLField field : idlClass.getFields()) {
            if (field.isIdlType()) {
                String type = field.getType();
                if (field.isArray()) {
                    type = type.substring(0, type.length() - 2);
                }
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
            ret += nonReservedName(idlClass.getEnumNames().get(i));
            if (i < idlClass.getEnumNames().size()-1) {
              ret += ",";
            }
        }
        return ret;
    }

    protected String getFieldName(IDLField field)
    {
        return nonReservedName(field.getName());
    }

    protected String getInitValue(IDLField field, IDLClass idlClass)
    {
      String fieldType = getLastPart(field.getType());
      fieldType = fieldType.replace("[]", "");
      if (fieldType.equals("string"))    return "null";
      if (fieldType.equals("boolean"))   return "False";
      if (fieldType.equals("double"))    return "0.0";
      if (fieldType.equals("float"))     return "0.0";
      if (field.isEnumType()) {
        //Set first enum value as init value
        if (field.getValue().length() > 0) {
            return nonReservedName(field.getValue());
        }
      }
      return "0";
    }

    protected String getDeclareVector(IDLField field, IDLClass idlClass)
    {
        String fieldType = getLastPart(field.getType());
        String ret = "";
        if (alwaysDynArray || (field.getArraySize() == 0)) {
            String typeStr, initStr = "", tickStr = "";
            // idl = type[] name;
            if (field.isIdlType() && (!field.isAbstract()) && (!alwaysDynObject)) {
                typeStr = elementType(fieldType) + "_Class_Arr";
            } else if (field.isIdlType()) {
                typeStr = elementType(fieldType) + "_Class_At_Arr";
                initStr = " => Create";
                tickStr = "'";
            } else {
                if (elementType(fieldType) == "String_At") {
                    typeStr = "String_Arr";
                    initStr = " => null";
                    tickStr = "'";
                } else {
                    typeStr = elementType(fieldType) + "_Arr";
                    initStr = " => " + getInitValue(field, idlClass);
                    tickStr = "'";
                }
            }
            if (field.getArraySize() == 0) {
                ret += typeStr + "_At := null;" + endl();
            } else {
                ret += typeStr + "_At := new " + typeStr + tickStr + "(0.." + (field.getArraySize() - 1) + initStr + ");" + endl();
            }
        } else {
            // idl = type[size] name;
            String arrDecl = "_Arr(0.." + (field.getArraySize() - 1) + ")";
            if (field.isIdlType() && (!field.isAbstract()) && (!alwaysDynObject)) {
                ret += elementType(fieldType) + "_Class" + arrDecl + ";" + endl();
            } else if(field.isIdlType()) {
                ret += elementType(fieldType) + "_Class_At" + arrDecl + " := (others => null);" + endl();
            } else {
                if (elementType(fieldType) == "String_At") {
                    ret += "String" + arrDecl + " := (others => " + getInitValue(field, idlClass) + ");" + endl();
                } else {
                    ret += elementType(fieldType) + arrDecl + " := (others => " + getInitValue(field, idlClass) + ");" + endl();
                }
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
            ret += tab(3) + idlClass.getClassName() + "_version : Byte := " + idlClass.getClassName() + "_idlVersion;" + endl();
        }
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            if(!field.getComment().equals("")) {
                String comment = field.getComment();
                int idx;
                while ((idx = comment.indexOf('\n')) >= 0) {
                  ret += tab(3) + "---" + comment.substring(0,idx).replace("/*", "").replace("*/", "") + endl();
                  comment = comment.substring(idx+1);
                }
                ret += tab(3) + "---" + comment.replace("/*", "").replace("*/", "") + endl();
            }
            String vers = getVersionDescription(field.getDirective());
            if (vers.length() > 0) {
                ret += tab(3) + "--- " + vers + endl();
            }
            String fieldType = getLastPart(field.getType());
            ret += tab(3) + getFieldName(field) + " : ";
            if (field.isArray()) {
                ret += getDeclareVector(field, idlClass);
            } else if ((!alwaysDynObject) && field.isIdlType() && !field.isAbstract()) {
                ret += "aliased " + languageType(fieldType) + "_Class;" + endl();
            } else if (field.isIdlType()) {
                ret += languageType(fieldType) + "_Class_At := null;" + endl();
            } else {
                ret += languageType(fieldType) + " := " + getInitValue(field, idlClass) + ";" + endl();
            }
        }
        if (ret == "") ret = tab(3) + "null;";
        return ret;
    }

    protected String getConstantDeclarations(IDLClass idlClass)
    {
      String ret = "";
      if (!isOnlyDefinition(idlClass)) {
          int version = idlClass.getVersion();
          if (version < 0) { version = 0; }
          ret += tab(1) + idlClass.getClassName() + "_idlVersion : constant Byte := " + version + ";" + endl();
      }
      for (IDLField field : idlClass.getFields()) {
          if (!field.isStatic()) continue;
          if(!field.getComment().equals("")) {
              String comment = field.getComment();
              int idx;
              while ((idx = comment.indexOf('\n')) >= 0) {
                ret += tab(1) + "---" + comment.substring(0,idx).replace("/*", "").replace("*/", "") + endl();
                comment = comment.substring(idx+1);
              }
              ret += tab(1) + "---" + comment.replace("/*", "").replace("*/", "") + endl();
          }

          String fieldType = languageType(getLastPart(field.getType()));
          if (fieldType.equals("String_At")) fieldType = "String";
          ret += tab(1) + getFieldName(field) + " : constant ";
          ret += fieldType + " := " + field.getValue() + ";" + endl();
      }
      return ret;
    }

    protected String getEnumTypeDeclarations(IDLClass idlClass)
    {
      String ret = "";
      for (IDLEnumType et : idlClass.getEnumTypes()) {
          if (!et.getComment().equals("")) {
              String comment = et.getComment();
              int idx;
              while ((idx = comment.indexOf('\n')) >= 0) {
                ret += tab(1) + "---" + comment.substring(0,idx).replace("/*", "").replace("*/", "") + endl();
                comment = comment.substring(idx+1);
              }
              ret += tab(1) + "---" + comment.replace("/*", "").replace("*/", "") + endl();
          }
          ret += tab(1) + "type " + et.getName() + " is (" + endl();
          String values = "";
          for (String eName : et.getEnumNames()) {
              if (values != "") values += ", ";
              values += nonReservedName(eName);
          }
          ret += tab(4) + values + endl();
          ret += tab(3) + ");" + endl();

          //type Order_Arr is array(Integer range <>) of aliased Order;
          //type Order_Arr_At is access all Order_Arr;
          ret += tab(1) + "type " + et.getName() + "_Arr is array(Integer range <>) of aliased " + et.getName() + ";" + endl();
          ret += tab(1) + "type " + et.getName() + "_Arr_At is access all " + et.getName() + "_Arr;" + endl();

          //procedure Dispose is new Ada.Unchecked_Deallocation( Order_Arr, Order_Arr_At );
          ret += tab(1) + "procedure Dispose is new Ada.Unchecked_Deallocation( " + et.getName() + "_Arr, " + et.getName() + "_Arr_At );" + endl();

          // procedure inout is new inoutenum(Order);
          // procedure inout is new inoutenumdynarr(Order, Order_Arr, Order_Arr_At, inout);
          ret += tab(1) + "procedure inout is new inoutenum(" + et.getName() + ");" + endl();
          ret += tab(1) + "procedure inout is new inoutenumdynarr(" + et.getName() + ", " + et.getName() + "_Arr, " + et.getName() + "_Arr_At, inout);" + endl() + endl();
      }
      return ret;
    }

//    protected String getValidationHead(IDLClass idlClass)
//    {
//      String ret = "";
//      for (IDLField field : idlClass.getFields())
//      {
//          if (field.isIdlType() && !field.isAbstract()) {
//              if (field.isArray()) {
//                  ret += tab(0) + "var" + endl();
//                  ret += tab(1) +   "__i__ : Integer;" + endl();
//                  break;
//              }
//          }
//      }
//      return ret;
//    }

//    protected String getValidationBody(IDLClass idlClass)
//    {
//      String ret = "";
//      for (IDLField field : idlClass.getFields())
//      {
//          String fieldType = getLastPart(field.getType());
//          String fieldName = getFieldName(field);
//          if (field.isIdlType() && !field.isAbstract())
//          {
//              // 'virtual': All fields that are objects are also virtual in Delphi!!
//              // Need to validate that an object that isn't declared 'virtual' really
//              // is of the correct type
//              if (field.isArray()) {
//                String s = field.getType();
//                s = getLastPart(s.substring(0, s.indexOf('[')));
//                ret += tab(1) + "for __i__ := 0 to High(" + fieldName + ") do begin" + endl();
//                ret += tab(2) + "if not " + fieldName + "[__i__].ClassNameIs('" + s + "') then Result := False;" + endl();
//                ret += tab(1) + "end;" + endl();
//              } else {
//                ret += tab(1) + "if not " + fieldName + ".ClassNameIs('" + getLastPart(field.getType()) + "') then Result := False;" + endl();
//              }
//          }
//      }
//      return ret;
//    }

    private String elementType(String type)
    {
        return languageType(type.replace("[]", ""));
    }

    protected String languageType(String s)
    {
      if (s.equals("string"))    return "String_At";
      if (s.equals("boolean"))   return "Boolean";
      if (s.equals("int"))       return "Int32";
      if (s.equals("short"))     return "Int16";
      if (s.equals("long"))      return "Int64";
      if (s.equals("double"))    return "Float64";
      if (s.equals("float"))     return "Float32";
      if (s.equals("byte"))      return "Byte";
      if (s.equals("string[]"))  return "String_Arr";
      if (s.equals("int[]"))     return "Int32_Arr";
      if (s.equals("short[]"))   return "Int16_Arr";
      if (s.equals("long[]"))    return "Int64_Arr";
      if (s.equals("double[]"))  return "Float64_Arr";
      if (s.equals("float[]"))   return "Float32_Arr";
      if (s.equals("byte[]"))    return "Byte_Arr";
      if (s.equals("boolean[]")) return "Boolean_Arr";
      return s;
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
        String versionName = idlClass.getClassName() + "_version";
        String versionNameIdl = idlClass.getClassName() + "_idlVersion";
        // Need an implicit version field that may be [de]serialized
        ret += tab(2) + "if Self.IdlVersionMask /= 0 then" + endl();
        ret += tab(3) + "archiver.Inout(\"" + versionName + "\", Self." + versionName + ");" + endl();
        ret += tab(3) + "ValidateVersion(\"" + idlClass.getClassName() + "\", Self." + versionName + ", " + versionNameIdl + ");" + endl();
        ret += tab(2) + "else" + endl();
        ret += tab(3) + "Self." + versionName + " := 0;" + endl();
        ret += tab(2) + "end if;" + endl();
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            String fieldName = getFieldName(field);
            String fieldGuard = getFieldGuard("Self." + versionName, field);
            int tabCnt = 2;
            if (fieldGuard.length() > 0) {
                ret += tab(tabCnt) + "if " + fieldGuard + " then" + endl();
                tabCnt += 1;
            }
            if (field.isIdlType()) {
                if (field.isArray()) {
                    ret += tab(tabCnt);
                    if ((!alwaysDynArray) && (field.getArraySize() > 0)) {
                        // idl = type[size] name;
                        // TestData_Class_InoutFixArr(archiver, "ftest2s2", Self.ftest2s2);
                        ret += elementType(getLastPart(field.getType())) + "_Class_InoutFixArr(archiver, \"" + field.getName() + "\", Self." + fieldName + ");" + endl();
                    } else {
                        String isAbsStr = "False";
                        if (field.isAbstract()) {
                            isAbsStr = "True";
                        }
                        ret += elementType(getLastPart(field.getType())) + "_Class_InoutDynArr(archiver, \"" + field.getName() + "\", Self." +
                                fieldName + ", " + isAbsStr + ");" + endl();
                    }
                } else {
                    if (alwaysDynObject || field.isAbstract()) {
                        ret += tab(tabCnt);
                        if (field.isAbstract()) {
                          ret += "Self." + fieldName + " := " + getFullyQualifiedClassName(field.getType()) + "_At" +
                               "(archiver.Inout2(\"" + field.getName() + "\", Serializable_Class_At(Self." + fieldName + ")));" + endl();
                        } else {
                          ret += "archiver.Inout(\"" + field.getName() + "\", Serializable_Class_At(Self." + fieldName + "));" + endl();
                        }
                    } else {
                        ret += tab(tabCnt) + "declare" + endl();
                        ret += tab(tabCnt+1) + "tmp : " + getFullyQualifiedClassName(field.getType()) + "_At := Self." + fieldName + "'Unchecked_Access;" + endl();
                        ret += tab(tabCnt) + "begin" + endl();
                        ret += tab(tabCnt+1) + "archiver.Inout(\"" + field.getName() + "\", Serializable_Class_At(tmp));" + endl();
                        ret += tab(tabCnt) + "end;" + endl();
                    }
                }
            } else if (field.isEnumType()) {
                // Enum types
                // Inout(archiver, "enu1", Self.enu1);
                ret += tab(tabCnt);
                ret += "Inout(archiver, \"" + field.getName() + "\", Self." + fieldName + ");" + endl();

            } else {
                ret += tab(tabCnt);
                // core types
                if (field.isArray()) {
                    if ((!alwaysDynArray) && (field.getArraySize() > 0)) {
                        // idl = type[size] name;
                        ret += "archiver.Inout(\"" + field.getName() + "\", Self." + fieldName + ");" + endl();
                    } else {
                        // idl = type[] name;
                        ret += "archiver.Inout(\"" + field.getName() + "\", Self." + fieldName + ");" + endl();
                    }
                } else {
                    ret += "archiver.Inout(\"" + field.getName() + "\", Self." + fieldName + ");" + endl();
                }
            }
            if (fieldGuard.length() > 0) {
                tabCnt -= 1;
                ret += tab(tabCnt) + "end if;" + endl();
            }
        }
        return ret;
    }

    protected String nonReservedName(String name)
    {
        if (isReservedName(name)) name = name + "_";
        // Ada don't allow leading and trailing "_" so change them
        if (name.charAt(0) == '_') name = "a" + name;
        if (name.charAt(name.length()-1) == '_') name = name + "a";
        return name;
    }

    public boolean isReservedName(String name)
    {
        return Arrays.binarySearch(reservedNames, name.toLowerCase()) >= 0;
    }

    // Array of all reserved keywords in ascending order (for binarySearch() to work)
    private static final String[] reservedNames = {
      "abort", "abs", "abstract", "accept", "access", "aliased", "all", "and",
      "array", "at",
      "begin", "body",
      "case", "constant",
      "declare", "delay", "delta", "digits", "do",
      "else", "elsif", "end", "entry", "exception", "exit",
      "for", "function",
      "generic", "goto",
      "if", "in", "interface", "is",
      "limited", "loop",
      "mod",
      "new", "not", "null",
      "of", "or", "others", "out", "overriding",
      "package", "pragma", "private", "procedure", "protected",
      "raise", "range", "record", "rem", "renames", "requeue", "return", "reverse",
      "select", "separate", "some", "subtype", "synchronized",
      "tagged", "task", "terminate", "then", "type",
      "until", "use",
      "when", "while", "with",
      "xor"
    };

}
