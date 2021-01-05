/** -*-java-*- Copyright (C) 2014 Saab Dynamics AB
 ***************************************************************************
 *  @file   opsc/CppCompiler.java
 *  @author Mattias Helsing <mattias.helsing@saabgroup.com>
 *          Updated/Modified by Lennart Andersson
 *
 * This file is based on:
 *   Tools/NBOPSIDLSupport/src/ops/netbeansmodules/idlsupport/CppCompiler.java
 * It is a complete copy of the said file but with dependencies to NB
 * completely removed. I have also added support for runtime configuration
 * of where output should go and where templates can be found
 *
 ***************************************************************************
 */

package opsc;

import java.io.IOException;
import java.io.InputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.Vector;
import java.util.Arrays;
import parsing.AbstractTemplateBasedIDLCompiler;
import parsing.IDLClass;
import parsing.IDLField;
import parsing.IDLEnumType;

/**
 *
 * @author angr
 */
public class CppCompiler extends opsc.Compiler
{
    final static String CONSTRUCTOR_BODY_REGEX = "__constructorBody";
    final static String CONSTRUCTOR_HEAD_REGEX = "__constructorHead";
    final static String DESTRUCTOR_BODY_REGEX = "__destructorBody";
    final static String DECLARATIONS_REGEX = "__declarations";
    final static String SERIALIZE_REGEX = "__serialize";
    final static String CLONE_REGEX = "__clone";
    final static String FILL_CLONE_REGEX = "__fillClone";
    final static String UNDERSCORED_PACK_NAME_REGEX = "__underscoredPackName";
    final static String PACKAGE_DECLARATION_REGEX = "__packageDeclaration";
    final static String PACKAGE_CLOSER_REGEX = "__packageCloser";
    final static String IMPORTS_REGEX = "__imports";
    final static String CLASS_COMMENT_REGEX = "__classComment";
    final static String SIZE_REGEX = "__size";
    private static String BASE_CLASS_NAME_REGEX = "__baseClassName";
    private static String CREATE_BODY_REGEX = "__createBody";
    private static String MEMORYPOOL_DECL_REGX = "__memoryPoolDecl";

    //private String projectDirectory;
    String createdFiles = "";

    public CppCompiler(String projectName) {
        super(projectName);
    }

    public void compileDataClasses(Vector<IDLClass> idlClasses, String projectDirectory)
    {
        //this.projectDirectory = projectDirectory;
        for (IDLClass iDLClass : idlClasses) {
            if (!_onlyGenTypeSupport) compileIDLClass(iDLClass);
            _idlClasses.add(iDLClass);
        }

        compileTypeSupport(_idlClasses, _projectName);
    }

    public void compileIDLClass(IDLClass idlClass)
    {
        try {
            if (idlClass.getType() == IDLClass.ENUM_TYPE) {
                // System.out.println("Compile enum");
                compileEnum(idlClass);
            } else {
                compileDataClass(idlClass);
                if (!isOnlyDefinition(idlClass)) {
                    if (!isTopLevel(idlClass)) {
                        System.out.println("Info: Cpp, skipping generation of publisher/subscriber for " + idlClass.getClassName());
                    } else {
                        compileSubscriber(idlClass);
                        compilePublisher(idlClass);
                    }
                }
            }
        } catch (IOException ioe) {
            System.out.println("Failed to generate code for " + idlClass.getClassName());
            System.out.println("  Generating C++ failed with the following exception: " + ioe.getMessage());
        }
    }

    private String getClassComment(IDLClass idlClass)
    {
        String ret = "";
        if (idlClass.getComment() != null) {
            if (!idlClass.getComment().equals("")) {
                String comment = idlClass.getComment();
                int idx;
                while ((idx = comment.indexOf('\n')) >= 0) {
                    ret += tab(0) + "///" + comment.substring(0,idx).replace("/*", "").replace("*/", "").replace("\r", "") + endl();
                    comment = comment.substring(idx+1);
                }
                ret += tab(0) + "///" + comment.replace("/*", "").replace("*/", "") + endl();
            }
        }
        return ret;
    }

    public void compileDataClass(IDLClass idlClass) throws IOException
    {
        String className = getClassName(idlClass);
        String baseClassName = "ops::OPSObject";
        if (idlClass.getBaseClassName() != null) {
            baseClassName = idlClass.getBaseClassName();
        }
        String packageName = idlClass.getPackageName();

        String packageFilePart = packageName.replace(".", "/");
        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + className + ".h");

        java.io.InputStream stream;
        if (isOnlyDefinition(idlClass)) {
            stream = findTemplateFile("cpptemplatebare.tpl");
        } else {
            stream = findTemplateFile("cpptemplate.tpl");
        }
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(CLASS_COMMENT_REGEX, getClassComment(idlClass));
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
        templateText = templateText.replace(BASE_CLASS_NAME_REGEX, applyLanguagePackageSeparator(baseClassName));
        templateText = templateText.replace(UNDERSCORED_PACK_NAME_REGEX, getUnderscoredPackName(packageName));
        templateText = templateText.replace(PACKAGE_DECLARATION_REGEX, getPackageDeclaration(packageName));
        templateText = templateText.replace(PACKAGE_CLOSER_REGEX, getPackageCloser(packageName));
        templateText = templateText.replace(IMPORTS_REGEX, getImports(idlClass));
        templateText = templateText.replace(CONSTRUCTOR_HEAD_REGEX, getConstructorHead(idlClass));
        templateText = templateText.replace(CONSTRUCTOR_BODY_REGEX, getConstructorBody(idlClass));
        templateText = templateText.replace(DESTRUCTOR_BODY_REGEX, getDestructorBody(idlClass));
        templateText = templateText.replace(DECLARATIONS_REGEX, getDeclarations(idlClass));
        templateText = templateText.replace(SERIALIZE_REGEX, getSerialize(idlClass));
        templateText = templateText.replace(CLONE_REGEX, getClone(idlClass));
        templateText = templateText.replace(FILL_CLONE_REGEX, getFillClone(idlClass));
        templateText = templateText.replace(MEMORYPOOL_DECL_REGX, getMemoryPoolDeclaration(idlClass));

        //Save the modified text to the output file.
        saveOutputText(templateText);

        createdFiles += "\"" + getOutputFileName() + "\"\n";
    }

    protected void compileEnum(IDLClass idlClass) throws IOException
    {
        String className = getClassName(idlClass);

        String packageName = idlClass.getPackageName();
        String packageFilePart = packageName.replace(".", "/");
        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + className + ".h");

        java.io.InputStream stream = findTemplateFile("cppenumtemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();
        //Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(CLASS_COMMENT_REGEX, getClassComment(idlClass));
        templateText = templateText.replace(PACKAGE_DECLARATION_REGEX, getPackageDeclaration(packageName));
        templateText = templateText.replace(PACKAGE_CLOSER_REGEX, getPackageCloser(packageName));
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
        templateText = templateText.replace(UNDERSCORED_PACK_NAME_REGEX, getUnderscoredPackName(packageName));
        templateText = templateText.replace(MEMORYPOOL_DECL_REGX, getMemoryPoolDeclaration(idlClass));

        templateText = templateText.replace(DECLARATIONS_REGEX, getEnumDeclarations(idlClass));

        //Save the modified text to the output file.
        saveOutputText(templateText);
        createdFiles += "\"" + getOutputFileName() + "\"\n";
    }

    private String getMemoryPoolDeclaration(IDLClass idlClass)
    {
        String ret = "";
        if (_genMemoryPool) {
            ret += tab(1) + "typedef ops::memory_pools::memory_pool_nd<" + getClassName(idlClass) + "> memory_pool_type;" + endl();
            ret += endl();
            ret += tab(1) + "void* operator new(size_t size) { return _pool.getEntry(size); }" + endl();
          	ret += tab(1) + "void operator delete(void *p) { _pool.returnEntry(p); }" + endl();
            ret += endl();
            ret += tab(0) + "private:" + endl();
          	ret += tab(1) + "static memory_pool_type _pool;" + endl();
            ret += endl();
            ret += tab(1) + "/// To define the pool, you need to add the following to a cpp-file:" + endl();
            ret += tab(1) + "///   " + getClassName(idlClass) + "::memory_pool_type " + getClassName(idlClass) + "::_pool(<max_number_of_objects>);";
        }
        return ret;
    }

    private String getEnumDeclarations(IDLClass idlClass)
    {
        String ret = "";
        for (int i = 0; i < idlClass.getEnumNames().size(); i++) {
            ret += tab(1) + "const static int " + idlClass.getEnumNames().get(i) + " = " + i + ";" + endl();
        }
        ret += tab(1) + "const static int UNDEFINED = " + idlClass.getEnumNames().size() + ";" + endl();
        return ret;
    }

    public String getName()
    {
        return "CppCompiler";
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

    protected void compilePublisher(IDLClass idlClass) throws IOException
    {
        String className = getClassName(idlClass);
        String packageName = idlClass.getPackageName();

        String packageFilePart = packageName.replace(".", "/");
        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + className + "Publisher.h");

        java.io.InputStream stream = findTemplateFile("cpppublishertemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
        templateText = templateText.replace(UNDERSCORED_PACK_NAME_REGEX, getUnderscoredPackName(packageName));
        templateText = templateText.replace(PACKAGE_DECLARATION_REGEX, getPackageDeclaration(packageName));
        templateText = templateText.replace(PACKAGE_CLOSER_REGEX, getPackageCloser(packageName));

        //Save the modified text to the output file.
        saveOutputText(templateText);

        createdFiles += "\"" + getOutputFileName() + "\"\n";
    }

    protected void compileSubscriber(IDLClass idlClass) throws IOException
    {
        String className = getClassName(idlClass);
        String packageName = idlClass.getPackageName();

        String packageFilePart = packageName.replace(".", "/");
        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + className + "Subscriber.h");

        java.io.InputStream stream = findTemplateFile("cppsubscribertemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
        templateText = templateText.replace(UNDERSCORED_PACK_NAME_REGEX, getUnderscoredPackName(packageName));
        templateText = templateText.replace(PACKAGE_DECLARATION_REGEX, getPackageDeclaration(packageName));
        templateText = templateText.replace(PACKAGE_CLOSER_REGEX, getPackageCloser(packageName));

        //Save the modified text to the output file.
        saveOutputText(templateText);

        createdFiles += "\"" + getOutputFileName() + "\"\n";
    }

    protected void compileTypeSupport(Vector<IDLClass> idlClasses, String projectName)
    {
      try {
        String className = projectName + "TypeFactory";
        String packageName = projectName;

        String packageFilePart = packageName.replace(".", "/");
        setOutputFileName(_outputDir + File.separator + projectName + File.separator + className + ".h");

        java.io.InputStream stream = findTemplateFile("cpptypefactorytemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
        templateText = templateText.replace(UNDERSCORED_PACK_NAME_REGEX, getUnderscoredPackName(packageName));
        templateText = templateText.replace(PACKAGE_DECLARATION_REGEX, getPackageDeclaration(packageName));
        templateText = templateText.replace(PACKAGE_CLOSER_REGEX, getPackageCloser(packageName));

        //Save the modified text to the output file.

        String createBodyText = "";
        String includes = "";

        for (IDLClass iDLClass : idlClasses) {
            if (isOnlyDefinition(iDLClass) || isNoFactory(iDLClass)) continue;

            createBodyText += tab(2) + "if(type == \"" + iDLClass.getPackageName() + "." + getClassName(iDLClass) + "\")" + endl();
            createBodyText += tab(2) + "{" + endl();
            createBodyText += tab(3) +   "return new " + applyLanguagePackageSeparator(iDLClass.getPackageName()) + "::" + getClassName(iDLClass) + "();" + endl();
            createBodyText += tab(2) + "}" + endl();

            includes += tab(0) + "#include \"" + getSlashedType(iDLClass.getPackageName()) + "/" + getSlashedType(getClassName(iDLClass)) + ".h\"" + endl();
        }
        createBodyText += tab(2) + "return nullptr;" + endl();

        templateText = templateText.replace(CREATE_BODY_REGEX, createBodyText);
        templateText = templateText.replace(IMPORTS_REGEX, includes);
        saveOutputText(templateText);

        createdFiles += "\"" + getOutputFileName() + "\"\n";
      } catch (IOException iOException) {
          System.out.println("Generating C++ Factory failed with the following exception: " + iOException.getMessage());
      }
    }

//    private String extractProjectName(String projectDirectory)
//    {
//        String projectName = projectDirectory.substring(0, projectDirectory.lastIndexOf("/Generated/"));
//        projectName = projectDirectory.substring(projectName.lastIndexOf("/") + 1, projectName.length());
//        return projectName;
//    }

    private String getClone(IDLClass idlClass)
    {
        String ret = tab(2) + getClassName(idlClass) + "* ret = new " + applyLanguagePackageSeparator(getClassName(idlClass)) + ";" + endl();
        ret += tab(2) + "fillClone(ret);" + endl();
        ret += tab(2) + "return ret;" + endl();

        return ret;
    }

    private String getFillClone(IDLClass idlClass)
    {
        String ret = "";

        if (idlClass.getBaseClassName() != null) {
            ret += tab(2) + applyLanguagePackageSeparator(idlClass.getBaseClassName()) + "::fillClone(obj);" + endl();
        } else {
            ret += tab(2) + "ops::OPSObject::fillClone(obj);" + endl();
        }
        ret += tab(2) + "obj->" + getClassName(idlClass) + "_version = " + getClassName(idlClass) + "_version;" + endl();
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            String fieldName = getFieldName(field);
            if (field.isIdlType()) {
                if (!field.isArray()) {
                    if (field.isAbstract()) {
                        ret += tab(2) + "if(obj->" + fieldName + ") delete obj->" + fieldName + ";" + endl();
                        ret += tab(2) + "obj->" + fieldName + " = (" + languageType(field) + "*)" + fieldName + "->clone();" + endl();
                    } else {
                        ret += tab(2) + "obj->" + fieldName + " = " + fieldName + ";" + endl();
                    }
                } else {
                    // isArray()
                    if (!field.isAbstract()) {
                        if (field.getArraySize() > 0) {
                            ret += tab(2) + "for(unsigned int __i = 0; __i < " + field.getArraySize() + "; __i++) {" + endl();
                            ret += tab(3) +   "obj->" + fieldName + "[__i] = " + fieldName + "[__i];" + endl();
                            ret += tab(2) + "}" + endl();
                        } else {
                            ret += tab(2) + "obj->" + fieldName + " = " + fieldName + ";" + endl();
                        }
                    } else {
                        if (field.getArraySize() > 0) {
                            // 		ops::cloneFixArrPtr<TestData, 5>(obj->ftest2s, ftest2s);
                            ret += tab(2) + "ops::cloneFixArrPtr<" + elementType(field) + ", " + field.getArraySize() + ">(obj->" + fieldName + ", " + fieldName + ");" + endl();
                        } else {
                            //		ops::cloneVectorPtr<TestData>(obj->test2s, test2s);
                            ret += tab(2) + "ops::cloneVectorPtr<" + elementType(field) + ">(obj->" + fieldName + ", " + fieldName + ");" + endl();
                        }
                    }
                }
            } else {
                // core types
                if (field.isArray()) {
                    if (field.getArraySize() > 0) {
                        if (!field.getType().equals("string[]")) {
                            ret += tab(2) + "memcpy(&obj->" + fieldName + "[0], &" + fieldName + "[0], sizeof(" + fieldName + "));" + endl();
                        } else {
                            ret += tab(2) + "for(unsigned int __i = 0; __i < " + field.getArraySize() + "; __i++) {" + endl();
                            ret += tab(3) +   "obj->" + fieldName + "[__i] = " + fieldName + "[__i];" + endl();
                            ret += tab(2) + "}" + endl();
                        }
                    } else {
                        ret += tab(2) + "obj->" + fieldName + " = " + fieldName + ";" + endl();
                    }
                } else {
                    ret += tab(2) + "obj->" + fieldName + " = " + fieldName + ";" + endl();
                }
            }
        }
        return ret;
    }

    protected String getConstructorBody(IDLClass idlClass)
    {
        String ret = "";
        if (idlClass.getVersion() > 0) {
            ret += tab(2) + "idlVersionMask |= 1;" + endl();
        }
        for (IDLField field : idlClass.getFields()) {
            String fieldName = getFieldName(field);
            if (field.isIdlType() && !field.isArray() && field.isAbstract()) {
                ret += tab(2) + fieldName + " = new " + languageType(field).replace("*", "()") + ";" + endl();
            }
            if (field.isArray() && (field.getArraySize() > 0)) {
                if (field.isIdlType()) {
                    if (field.isAbstract()) {
                        ret += tab(2) + "for(unsigned int __i = 0; __i < " + field.getArraySize() + "; __i++) {" + endl();
                        ret += tab(3) +   fieldName + "[__i] = new " + languageType(field).replace("*", "()") + ";" + endl();
                        ret += tab(2) + "}" + endl();
                    }
                } else if (field.isEnumType()) {
                    if (field.getValue().length() > 0) {
                      ret += tab(2) + "for(unsigned int __i = 0; __i < " + field.getArraySize() + "; __i++) {" + endl();
                      ret += tab(3) +   fieldName + "[__i] = " + languageType(field) + "::" + field.getValue() + ";" + endl();
                      ret += tab(2) + "}" + endl();
                    }
                } else {
                    if (!field.getType().equals("string[]")) {
                        // int kalle[356]; --> memset(&kalle[0], 0, sizeof(kalle));
                        ret += tab(2) + "memset(&" + fieldName + "[0], 0, sizeof(" + fieldName + "));" + endl();
                    }
                }
            }
        }
        return ret;
    }

    private String getConstructorHead(IDLClass idlClass)
    {
        String ret = tab(2) + "";
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            String fieldName = getFieldName(field);
            if (field.getType().equals("boolean")) {
                ret += ", " + fieldName + "(false)";
            } else if (field.getType().equals("string") || field.isArray() || field.isIdlType()) {
                //Do nothing in head
            } else if (field.isEnumType()) {
                if (field.getValue().length() > 0) {
                    ret += ", " + fieldName + "(" + languageType(field) + "::" + field.getValue() + ")";
                }
            } else {
                //Numeric
                ret += ", " + fieldName + "(0)";
            }
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
                  ret += tab(1) + "///" + comment.substring(0,idx).replace("/*", "").replace("*/", "").replace("\r", "") + endl();
                  comment = comment.substring(idx+1);
              }
              ret += tab(1) + "///" + comment.replace("/*", "").replace("*/", "") + endl();
          }
          ret += tab(1) + "enum class " + et.getName() + " {" + endl();
          String values = "";
          for (String eName : et.getEnumNames()) {
              if (values != "") values += ", ";
              values += eName;
          }
          ret += tab(2) + values + endl();
          ret += tab(1) + "};" + endl();
      }
      if (ret != "") ret += endl();
      return ret;
    }

    protected String getDeclarations(IDLClass idlClass)
    {
        String ret = getEnumTypeDeclarations(idlClass);

        if (!isOnlyDefinition(idlClass)) {
            int version = idlClass.getVersion();
            if (version < 0) { version = 0; }
            // Need an implicit version field that should be [de]serialized
            ret += tab(1) + "static const char " + getClassName(idlClass) + "_idlVersion = " + version + ";" + endl() + endl();
        }

        for (IDLField field : idlClass.getFields()) {
            String fieldName = getFieldName(field);
            if (!field.getComment().equals("")) {
                String comment = field.getComment();
                int idx;
                while ((idx = comment.indexOf('\n')) >= 0) {
                  ret += tab(1) + "///" + comment.substring(0,idx).replace("/*", "").replace("*/", "").replace("\r", "") + endl();
                  comment = comment.substring(idx+1);
                }
                ret += tab(1) + "///" + comment.replace("/*", "").replace("*/", "") + endl();
            }
            if (field.isArray()) {
                ret += tab(1) + "" + getDeclareVector(field);
            } else {
                if (field.getType().equals("string")) {
                    if (field.isStatic()) {
                        // NOTE strings can't be const in header file, so use a trick with an inline method
                        ret += tab(1) + "static const char* " + fieldName + "() { return " + field.getValue() + "; }" + endl();
                    } else {
                        ret += tab(1) + "" + languageType(field) + " " + fieldName + ";" + endl();
                    }
                } else {
                    if (field.isIdlType()) {
                        if (field.isAbstract()) {
                            ret += tab(1) + "" + languageType(field) + "* " + fieldName + ";" + endl();
                        } else {
                            ret += tab(1) + "" + languageType(field) + " " + fieldName + ";" + endl();
                        }
                    } else {
                        //Simple primitive type
                        if (field.isStatic()) {
                            if (languageType(field).equals("float") || languageType(field).equals("double")) {
                                // NOTE floating point numbers can't be const in header file, so use a trick with an inline method
                                String pf = "";
                                if (languageType(field).equals("float")) pf = "f";
                                ret += tab(1) + "static const " + languageType(field) + " " + fieldName + "() { return " + field.getValue() + pf + "; }" + endl();
                            } else {
                                ret += tab(1) + "static const " + languageType(field) + " " + fieldName + " = " + field.getValue() + ";" + endl();
                            }
                        } else {
                            ret += tab(1) + "" + languageType(field) + " " + fieldName + ";" + endl();
                        }
                    }
                }
            }
        }
        return ret;
    }

    protected String getDeclareVector(IDLField field)
    {
        String ret = "";
        String fieldName = getFieldName(field);
        if (field.getArraySize() == 0) {
          // idl = type[] name;
          ret += "std::vector<" + elementType(field);
          if (field.isAbstract()) ret += "*";
          ret += "> " + fieldName + ";" + endl();
        } else {
          // idl = type[size] name;
          ret += elementType(field);
          if (field.isAbstract()) ret += "*";
          ret += " " + fieldName + "[" + field.getArraySize() + "];" + endl();
        }
        return ret;
    }

    private String elementType(IDLField field)
    {
        return languageType(field);
    }

    protected String languageType(IDLField field)
    {
        String s = field.getType().replace("[]", "");
        if (s.equals("string") && (field.getStringSize() > 0))  return "ops::strings::fixed_string<" + field.getStringSize() + ">";
        if (s.equals("string") && (field.getStringSize() == 0)) return "std::string";
        if (s.equals("boolean"))                                return "bool";
        if (s.equals("int"))                                    return "int";
        if (s.equals("short"))                                  return "short";
        if (s.equals("long"))                                   return "int64_t";
        if (s.equals("double"))                                 return "double";
        if (s.equals("float"))                                  return "float";
        if (s.equals("byte"))                                   return "char";
        return applyLanguagePackageSeparator(s);
    }

    protected String languageType(String s)
    {
//      if (s.equals("string"))    return "std::string_XXX";
//      if (s.equals("boolean"))   return "bool_XXX";
//      if (s.equals("int"))       return "int_XXX";
//      if (s.equals("short"))     return "short_XXX";
//      if (s.equals("long"))      return "__int64_XXX";
//      if (s.equals("double"))    return "double_XXX";
//      if (s.equals("float"))     return "float_XXX";
//      if (s.equals("byte"))      return "char_XXX";
//
//      if (s.equals("string[]"))  return "std::vector<std::string>_XXX";
//      if (s.equals("int[]"))     return "std::vector<int>_XXX";
//      if (s.equals("short[]"))   return "std::vector<short>_XXX";
//      if (s.equals("long[]"))    return "std::vector<__int64>_XXX";
//      if (s.equals("double[]"))  return "std::vector<double>_XXX";
//      if (s.equals("float[]"))   return "std::vector<float>_XXX";
//      if (s.equals("byte[]"))    return "std::vector<char>_XXX";
//      if (s.equals("boolean[]")) return "std::vector<bool>_XXX";
//
//      if (s.endsWith("[]"))      return "XXX_std::vector<" + applyLanguagePackageSeparator(s.substring(0, s.indexOf('['))) + "*>";
//      return applyLanguagePackageSeparator(s) + "_XXX";
        return "...NYI...";
    }

    protected String applyLanguagePackageSeparator(String packageName)
    {
        return packageName.replace(".", "::");
    }

    private CharSequence getImports(IDLClass idlClass)
    {
        HashMap<String, String> typesToInclude = new HashMap();
        String ret = "";
        if (idlClass.getBaseClassName() != null) {
            typesToInclude.put(idlClass.getBaseClassName(), idlClass.getBaseClassName());
        }
        for (IDLField field : idlClass.getFields()) {
            if (field.isIdlType()) {
                String type = field.getType();
                if (field.isArray()) {
                    type = type.substring(0, type.length() - 2);
                }
                typesToInclude.put(type, type);
            }
        }
        for (String s : idlClass.getImports()) {
            typesToInclude.put(s, s);
        }
        if(_genMemoryPool) {
            ret += tab(0) + "#include \"memory_pool.h\"" + endl();
        }
        for (String includeType : typesToInclude.values()) {
            ret += tab(0) + "#include \"" + getSlashedType(includeType) + ".h\"" + endl();
        }
        return ret;
    }

    private String getSlashedType(String type)
    {
        return type.replace(".", "/");
    }

    private String getUnderscoredPackName(String packageName)
    {
        return packageName.replaceAll("\\.", "_");
    }

    private CharSequence getDestructorBody(IDLClass idlClass)
    {
        String ret = "";
        for (IDLField field : idlClass.getFields()) {
            String fieldName = getFieldName(field);
            if (field.isIdlType() && field.isAbstract()) {
                ret += tab(2);
                if (!field.isArray()) {
                    ret += "if (" + fieldName + " != nullptr) { delete " + fieldName + "; }" + endl();
                } else {
                    if (field.getArraySize() > 0) {
                        ret += "for (unsigned int __i = 0; __i < ";
                        ret += field.getArraySize();
                        ret += "; __i++) { if (" + fieldName + "[__i] != nullptr) { delete " + fieldName + "[__i]; } }" + endl();
                    } else {
                        ret += "for (auto& x : " + fieldName + ") { if (x != nullptr) { delete x; } }" + endl();
                    }
                }
            }
        }
        return ret;
    }

    private String getPackageCloser(String packageName)
    {
        String ret = "";
        for (int i = 0; i < packageName.length(); i++) {
            if (packageName.charAt(i) == '.') {
                ret += "}";
            }
        }
        ret += "}\n";
        return ret;
    }

    private String getPackageDeclaration(String packageName)
    {
        String ret = "namespace " + packageName.replaceAll("\\.", " { namespace ");
        ret += " {";
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
                    cond = "(" + cond + " && (" + versionName + " <= " + ent.stop + "))";
                }
                if (ret.length() > 0) {
                    ret += " || ";
                }
                ret += cond;
            }
        }
        return ret;
    }

    protected String getSerialize(IDLClass idlClass)
    {
        String ret = tab(2);
        if (idlClass.getBaseClassName() != null) {
            ret += applyLanguagePackageSeparator(idlClass.getBaseClassName()).replace("*", "") + "::serialize(archive);" + endl();
        } else {
            ret += "ops::OPSObject::serialize(archive);" + endl();
        }
        String versionName = getClassName(idlClass) + "_version";
        String versionNameIdl = getClassName(idlClass) + "_idlVersion";
        // Need an implicit version field that may be [de]serialized
        ret += tab(2) + "if (idlVersionMask != 0) {" + endl();
        ret += tab(3) + "archive->inout(\"" + versionName + "\", " + versionName + ");" + endl();
        ret += tab(3) + "ValidateVersion(\"" + getClassName(idlClass) + "\", " + versionName + ", " + versionNameIdl + ");" + endl();
        ret += tab(2) + "} else {" + endl();
        ret += tab(3) + versionName + " = 0;" + endl();
        ret += tab(2) + "}" + endl();
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            String fieldName = getFieldName(field);
            String fieldGuard = getFieldGuard(versionName, field);
            int tabCnt = 2;
            if (fieldGuard.length() > 0) {
                ret += tab(tabCnt) + "if (" + fieldGuard + ") {" + endl();
                tabCnt += 1;
            }
            ret += tab(tabCnt);
            if (field.isIdlType()) {
                if (!field.isArray()) {
                    if (field.isAbstract()) {
                        ret += fieldName + " = (" + languageType(field) + "*) ";
                    }
                    ret += "archive->inout(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                } else {
                    // isArray()
                    if (field.getArraySize() > 0) {
                        // idl = type[size] name;
                        // template <class SerializableType> void inoutfixarr(InoutName_T name, SerializableType** value, int numElements) // for virtual
                        // template <class SerializableType> void inoutfixarr(InoutName_T name, SerializableType* value, int numElements)  // for non virtual
                        ret += "archive->inoutfixarr<" + elementType(field) +
                              ">(\"" + field.getName() + "\", &" + fieldName + "[0], " + field.getArraySize() + ");" + endl();
                    } else {
                        // idl = type[] name;
                        ret += "archive->inout<" + elementType(field) + ">(\"" + field.getName() + "\", " + fieldName;
                        if (field.isAbstract()) {
                            ret += ");" + endl();
                        } else {
                            ret += ", " + elementType(field) + "());" + endl();
                        }
                    }
                }
            } else if (field.isEnumType()) {
                // Enum types
                // archive->inoutenum<Order>("enu1", enu1);
                // archive->inoutenum<Order>("enuVec", enuVec);
                // archive->inoutenum<Order>("enuFixArr", enuFixArr, 6);
                String arrSize = "";
                if (field.isArray() && (field.getArraySize() > 0)) {
                    arrSize = ", " + field.getArraySize();
                }
                ret += "archive->inoutenum(\"" + field.getName() + "\", " + fieldName + arrSize + ");" + endl();

            } else {
                // core types
                if (field.isArray()) {
                    if (field.getArraySize() > 0) {
                        // idl = type[size] name;
                        ret += "archive->inoutfixarr(\"" + field.getName() + "\", ";
                        if (!field.getType().equals("string[]")) {
                            //void inoutfixarr(InoutName_T name, bool* value, int numElements, int totalSize);
                            //void inoutfixarr(InoutName_T name, ...* value, int numElements, int totalSize);
                            //void inoutfixarr(InoutName_T name, double* value, int numElements, int totalSize);
                            ret += "&" + fieldName + "[0], " + field.getArraySize() + ", sizeof(" + fieldName + "));" + endl();
                        } else {
                            //void inoutfixarr(InoutName_T name, std::string* value, int numElements)
                            ret += "&" + fieldName + "[0], " + field.getArraySize() + ");" + endl();
                        }
                    } else {
                        // idl = type[] name;
                        ret += "archive->inout(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                    }
                } else {
                    ret += "archive->inout(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                }
            }
            if (fieldGuard.length() > 0) {
                tabCnt -= 1;
                ret += tab(tabCnt) + "}" + endl();
            }
        }
        return ret;
    }

    public boolean isReservedName(String name)
    {
        return Arrays.binarySearch(reservedNames, name) >= 0;
    }

    // Array of all reserved keywords in ascending order (for binarySearch() to work)
    private static final String[] reservedNames = {
      "alignas", "alignof", "and", "and", "and_eq", "asm",
      "atomic_cancel", "atomic_commit", "atomic_noexcept", "auto",
      "bitand", "bitor", "bool", "break",
      "case", "catch", "char", "char16_t", "char32_t", "class", "compl",
      "concept", "const", "constexpr", "const_cast", "continue",
      "co_await", "co_return", "co_yield",
      "decltype", "default", "delete", "do", "double", "dynamic_cast",
      "else", "enum", "explicit", "export", "extern",
      "false", "float", "for", "friend",
      "goto",
      "if", "import", "inline", "int",
      "long",
      "module", "mutable",
      "namespace", "new", "noexcept", "not", "not_eq", "nullptr",
      "operator", "or", "or_eq",
      "private", "protected", "public",
      "register", "reinterpret_cast", "requires", "return",
      "short", "signed", "sizeof", "static", "static_assert", "static_cast",
      "struct", "switch", "synchronized",
      "template", "this", "thread_local", "throw", "true", "try", "typedef",
      "typeid", "typename",
      "union", "unsigned", "using",
      "virtual", "void", "volatile",
      "wchar_t", "while",
      "xor", "xor_eq"
    };

}
